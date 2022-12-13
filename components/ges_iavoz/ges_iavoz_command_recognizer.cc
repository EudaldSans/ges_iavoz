/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/


#include "ges_iavoz_command_recognizer.h"

#include <limits>
#include <iostream>
#include <iomanip>

RecognizeCommands::RecognizeCommands(tflite::ErrorReporter* error_reporter,
                                    int32_t average_window_duration_ms,
                                    uint8_t detection_threshold,
                                    uint8_t weak_detection_threshold,
                                    int32_t suppression_ms,
                                    int32_t minimum_count)
        : error_reporter_(error_reporter),
        average_window_duration_ms_(average_window_duration_ms),
        detection_threshold_(detection_threshold),
        weak_detection_threshold_(weak_detection_threshold),
        suppression_ms_(suppression_ms),
        minimum_count_(minimum_count),
        previous_results_(error_reporter){

    previous_top_label_ = IAVOZ_KEY_NULL;
    total_consecutive_tops_ = 0;
    accumulated_probability_ = 0;

    first_top_time_ = std::numeric_limits<int32_t>::min();
    previous_top_label_time_ = std::numeric_limits<int32_t>::min();

    activation = false; 
    weak_activation = false;
}

TfLiteStatus RecognizeCommands::ProcessLatestResults(
            const TfLiteTensor* latest_results, const int32_t current_time_ms,
            IAVOZ_KEY_t* found_command, uint8_t* score, bool* is_new_command, 
            uint8_t* found_index, bool* valid_command) {
                
    if ((latest_results->dims->size != 2) || (latest_results->dims->data[0] != 1) || (latest_results->dims->data[1] != kCategoryCount)) {
        TF_LITE_REPORT_ERROR(error_reporter_,
            "The results for recognition should contain %d elements, but there are %d in an %d-dimensional shape",
            kCategoryCount, latest_results->dims->data[1], latest_results->dims->size);
        return kTfLiteError;
    }

    if (latest_results->type != kTfLiteInt8) {
        TF_LITE_REPORT_ERROR(error_reporter_, "The results for recognition should be int8_t elements, but are %d", latest_results->type);
        return kTfLiteError;
    }

    if ((!previous_results_.empty()) && (current_time_ms < previous_results_.front().time_)) {
        TF_LITE_REPORT_ERROR(error_reporter_,
            "Results must be fed in increasing time order, but received a timestamp of %d that was earlier than the previous one of %d",
            current_time_ms, previous_results_.front().time_);
        return kTfLiteError;
    }

    // Add the latest results to the head of the queue.
    previous_results_.push_back({current_time_ms, latest_results->data.int8});

    // Prune any earlier results that are too old for the averaging window.
    const int64_t time_limit = current_time_ms - average_window_duration_ms_;
    while ((!previous_results_.empty()) && previous_results_.front().time_ < time_limit) {
        previous_results_.pop_front();
    }

    // If there are too few results, assume the result will be unreliable and
    // bail.
    const int64_t how_many_results = previous_results_.size();
    const int64_t earliest_time = previous_results_.front().time_;
    const int64_t samples_duration = current_time_ms - earliest_time;
    // if ((how_many_results < minimum_count_) || (samples_duration < (average_window_duration_ms_ / 4))) {
    //     *found_command = previous_top_label_;
    //     *score = 0;
    //     *is_new_command = false;
    //     printf("Too few results\n");
    //     return kTfLiteOk;
    // }

    // Calculate the average score across all the results in the window.
    int32_t average_scores[kCategoryCount];
    for (int offset = 0; offset < previous_results_.size(); ++offset) {
        PreviousResultsQueue::Result previous_result = previous_results_.from_front(offset);
        const int8_t* scores = previous_result.scores;
        for (int i = 0; i < kCategoryCount; ++i) {
            if (offset == 0) {
                average_scores[i] = scores[i] + 128;
            } else average_scores[i] += scores[i] + 128;
        }
    }
    for (int i = 0; i < kCategoryCount; ++i) {
        average_scores[i] /= how_many_results;
    }

    // Find the current highest scoring category.
    int current_top_index = 0;
    int32_t current_top_score = 0;
    int high_probability_samples = 0;
    for (int i = 0; i < kCategoryCount; ++i) {
        if (average_scores[i] > current_top_score) {
            current_top_score = average_scores[i];
            current_top_index = i;
        }
        if ((average_scores[i]) > 50) {high_probability_samples++;}
    }
    IAVOZ_KEY_t current_top_label = kCategoryLabels[current_top_index];

    // If we've recently had another label trigger, assume one that occurs too
    // soon afterwards is a bad result.
    int64_t time_since_last_top;
    if ((previous_top_label_ == kCategoryLabels[0]) || (previous_top_label_time_ == std::numeric_limits<int32_t>::min())) {
        time_since_last_top = std::numeric_limits<int32_t>::max();
    } else {
        time_since_last_top = current_time_ms - previous_top_label_time_;
    }

    std::cout << std::setprecision(2) << std::fixed;
    std::cout << "SCORES " << current_time_ms << "ms";
    for (int i = 0; i < kCategoryCount; ++i) 
    //To calculate % threshold
    std::cout << "\t " << kCategoryLabels[i] << ": " << 100*(((float)average_scores[i]) / ((float)253)) << "% ";
    std::cout << "\t top label: " << current_top_label << " " << 100*((float)current_top_score / (float)253) << "%";
    std::cout << std::endl;

    *is_new_command = false;

    if (time_since_last_top >= 2000 && activation) {
        reset_state(is_new_command);
        std::cout << "Timed out!" << std::endl;
    }

    if (current_top_label != previous_top_label_) {
        first_top_time_ = current_time_ms;
        total_consecutive_tops_ = 1;
        accumulated_probability_ = current_top_score;
    } else {
        total_consecutive_tops_ ++;
        accumulated_probability_ += current_top_score;
    }

    int32_t accum_prob = accumulated_probability_/total_consecutive_tops_;
    bool weak_detection = (accum_prob > weak_detection_threshold_) && total_consecutive_tops_ > 1;
    if (total_consecutive_tops_ > 3) {
        weak_activation = false;
        reset_state(is_new_command);
        return kTfLiteOk;
    } 

    if (current_top_label == IAVOZ_KEY_NULL)         {std::cout << "Not keyword!" << std::endl; return kTfLiteOk;}
    // if (time_since_last_top < suppression_ms_)       {std::cout << "Too many keywords together!" << std::endl; return kTfLiteOk;}
    // if (high_probability_samples != 1)               {std::cout << "High probability samples!" << std::endl; return kTfLiteOk;}
    if (current_top_score < detection_threshold_)    {std::cout << "Low top score!" << std::endl; return kTfLiteOk;}
    // if (current_top_label == previous_top_label_)    {return kTfLiteOk;}
    
    if (current_top_label == IAVOZ_KEY_HEYLOLA && !activation) {
        if (current_top_score < detection_threshold_) {weak_activation = true; std::cout << "Weak ";}
        
        activation = true;
        previous_top_label_time_ = current_time_ms;
        std::cout << "Activation" << std::endl;
        *valid_command = true;
    } else if (current_top_label != IAVOZ_KEY_HEYLOLA && current_top_label != IAVOZ_KEY_NULL && activation) {
        if (weak_activation && current_top_score < detection_threshold_) {return kTfLiteOk;}
        if (current_top_score < detection_threshold_) {std::cout << "Weak ";}

        *is_new_command = true;
        activation = false;
        previous_top_label_time_ = std::numeric_limits<int32_t>::min();
        std::cout << "Command " << current_top_label << std::endl;
        *valid_command = true;
    }

    // esp_event_post_to(events_conn_loop_h, EVENTS_AUDIO, EVENT_CONN_WEIGHTS_TRANSMISSION, average_scores, kCategoryCount, portMAX_DELAY);

    previous_top_label_ = current_top_label;

    *found_command = current_top_label;
    *score = current_top_score;
    *found_index = current_top_index;

    return kTfLiteOk;
}

void RecognizeCommands::reset_state(bool* is_new_command) {
    *is_new_command = false;
    activation = false;
    weak_activation = false;

    previous_top_label_ = IAVOZ_KEY_NULL;
    previous_top_label_time_ = std::numeric_limits<int32_t>::min();

    first_top_time_ = std::numeric_limits<int32_t>::min();
    total_consecutive_tops_ = 0;
    accumulated_probability_ = 0;
}

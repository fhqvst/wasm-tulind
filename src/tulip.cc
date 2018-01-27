#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "nbind/api.h"
#include "../tulip/indicators.h"

int indicator(
  std::string _name,
  std::vector<std::string> _data_in,
  std::vector<std::string> _options,
  nbind::cbFunction &callback
) {
  // Indicator info
  const ti_indicator_info *info = ti_indicators;

  // This will be the result
  std::vector<std::string> result;
  
  // Function name
  const char *name = _name.c_str();

  info = ti_find_indicator(name);
  if (!info) {
    callback.call<int>("Could not find indicator.", result);
    return 1;
  }

  // Convert input strings to doubles
  TI_REAL data_in[_data_in.size()];
  for (unsigned int i = 0; i < _data_in.size(); i++) {
    data_in[i] = std::stod(_data_in[i]);
  }

  // Length of input
  const int input_length = sizeof(data_in) / sizeof(double);

  // Convert option strings to doubles
  TI_REAL options[_options.size()];
  if (info->options != (int)_options.size()) {
    callback.call<int>("Wrong number of options.", result);
    return 1;
  }

  for (unsigned int i = 0; i < _options.size(); i++) {
    options[i] = std::stod(_options[i]);
  }

  // Start amount
  int start = info->start(options);

  // Length of output
  const int output_length = input_length - start;

  // Pointer to output data
  double *data_out = new double[output_length * sizeof(double)];

  double *all_inputs[] = {data_in};
  double *all_outputs[] = {data_out};

  const int ret = info->indicator(input_length, all_inputs, options, all_outputs);
  if (ret == TI_OKAY) {
    // Place result in string vector
    for (int i = 0; i < output_length; i++) {
      std::string out = std::to_string(data_out[i]);
      result.push_back(out);
    }
    callback.call<int>("", result);
    return 0;
  } else {
    if (ret == TI_INVALID_OPTION) {
      callback.call<int>("Invalid option.", result);
    } else {
      callback.call<int>("Unknown error.", result);
    }
    return 1;
  }
}

int indicators(nbind::cbFunction &callback) {
  // Set info to first indicators in array.
  const ti_indicator_info *info = ti_indicators;

  // The last item is all zeros, so we'll stop when we get there.
  std::vector<std::string> result;
  while (info->name != 0) {
    // printf("%s (%s) has type %d with: %d inputs, %d options, %d outputs.\n",
    //         info->name,
    //         info->full_name,
    //         info->type,
    //         info->inputs,
    //         info->options,
    //         info->outputs
    //       );
    
    result.push_back(info->name);

    // Next indicator
    ++info;
  }

  callback.call<int>("", result);
  return 0;
}

#include "nbind/nbind.h"

NBIND_GLOBAL() {
  function(indicators);
  function(indicator);
}

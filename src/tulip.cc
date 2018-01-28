#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "nbind/api.h"
#include "../tulipindicators/indicators.h"

class Indicator {

public:

  /* Properties */
  std::string name;
  std::string full_name;
  std::string type;
  int inputs;
  int options;
  int outputs;
  std::vector<std::string> input_names;
  std::vector<std::string> option_names;
  std::vector<std::string> output_names;

  /* Getters */
  std::string getName() { return(name); }
  std::string getFullName() { return(full_name); }
  std::string getType() { return(type); }
  int  getInputs() { return(inputs); }
  int  getOptions() { return(options); }
  int  getOutputs() { return(outputs); }
  std::vector<std::string> getInputNames() { return(input_names); }
  std::vector<std::string> getOptionNames() { return(option_names); }
  std::vector<std::string> getOutputNames() { return(output_names); }

  /* Setters */
  void setName(std::string name) { this->name = name; }
  void setFullName(std::string full_name) { this->full_name = full_name; }
  void setType(std::string type) { this->type = type; }
  void setInputs(int inputs) { this->inputs = inputs; }
  void setOptions(int options) { this->options = options; }
  void setOutputs(int outputs) { this->outputs = outputs; }
  void setInputNames(std::vector<std::string> &input_names) { this->input_names = input_names; }
  void setOptionNames(std::vector<std::string> option_names) { this->option_names = option_names; }
  void setOutputNames(std::vector<std::string> output_names) { this->output_names = output_names; }

};

/**
 * Run an indicator function.
 *
 * Because nbind 1.37.29 do not support vectors of integers, we let all parameters be strings.
 * 
 * @param  _index
 * @param  _inputs
 * @param  _options
 * @param  callback
 * @return int
 */
int indicator(std::string _index,
  std::vector<std::vector<std::string>> _inputs,
  std::vector<std::string> _options,
  nbind::cbFunction &callback
) {
  std::vector<std::string> result;

  TI_REAL **inputs;
  TI_REAL options[_options.size()];
  TI_REAL **outputs;

  int input_length = -1;
  int output_length = -1;

  // Lookup indicator
  const int index = std::stoi(_index);
  const ti_indicator_info *info = ti_indicators + index;

  if (!info) {
    callback.call<int>("Could not find indicator.", result);
    return 1;
  }

  /* Validate inputs */
  if (info->inputs != (int)_inputs.size()) {
    callback.call<int>("Invalid number of inputs.", result);
    return 1;
  }

  /* Setup inputs */
  inputs = new double* [_inputs.size()];
  for (int i = 0; i < (int)_inputs.size(); i++) {
    inputs[i] = new double[_inputs[i].size()];

    for (int j = 0; j < (int)_inputs[i].size(); j++) {
      inputs[i][j] = std::stod(_inputs[i][j]);
    }
  }

  /* Find smallest size of input */
  for (int i = 0; i < info->inputs; i++) {
    const int size = _inputs[i].size();
    if (input_length == -1 || size < input_length) {
      input_length = size;
    }
  }

  /* Validate options */
  if (info->options != (int)_options.size()) {
    callback.call<int>("Invalid number of options.", result);
    return 1;
  }

  /* Setup options */
  for (int i = 0; i < (int)_options.size(); i++) {
    options[i] = std::stod(_options[i]);
  }

  /* Calculate output length */
  output_length = input_length - info->start(options);

  /* Setup outputs */
  outputs = new double* [info->outputs];
  for (int i = 0; i < info->outputs; i++) {
    outputs[i] = new double[output_length];
  }

  /* Run the indicator */
  const int ret = info->indicator(input_length, inputs, options, outputs);

  /* De-allocate input arrays */
  for (int i = 0; i < (int)_inputs.size(); i++) {
    delete [] inputs[i];
  }
  delete [] inputs;

  if (ret == TI_OKAY) {

    /* Convert output to strings */
    for (int i = 0; i < info->outputs; i++) {
      for (int j = 0; j < output_length; j++) {
        std::string out = std::to_string(outputs[i][j]);
        result.push_back(out);
      }
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

/**
 * List all indicators
 * 
 * @param  callback
 * @return int
 */
int list_indicators(nbind::cbFunction &callback) {
  /* Set pointer to first indicator function */
  const ti_indicator_info *info = ti_indicators;

  /* The last item is all zeros, so we'll stop when we get there */
  std::vector<Indicator> result;
  while (info->name != 0) {
    Indicator indicator;
    indicator.setName(std::string(info->name));
    indicator.setFullName(std::string(info->full_name));
    indicator.setInputs(info->inputs);
    indicator.setOptions(info->options);
    indicator.setOutputs(info->outputs);

    char const *type = "unknown";

    switch (info->type) {
      case TI_TYPE_OVERLAY:
        type = "overlay";
        break;
      case TI_TYPE_INDICATOR:
        type = "indicator";
        break;
      case TI_TYPE_MATH:
        type = "math";
        break;
      case TI_TYPE_SIMPLE:
        type = "simple";
        break;
      case TI_TYPE_COMPARATIVE:
        type = "comparative";
        break;
    }

    indicator.setType(type);

    /* Set input names */
    std::vector<std::string> input_names;
    for (int i = 0; i < info->inputs; i++) {
      input_names.push_back(info->input_names[i]);
    }
    indicator.setInputNames(input_names);

    /* Set option names */
    std::vector<std::string> option_names;
    for (int i = 0; i < info->options; i++) {
      option_names.push_back(info->option_names[i]);
    }
    indicator.setOptionNames(option_names);

    /* Setup output names */
    std::vector<std::string> output_names;
    for (int i = 0; i < info->outputs; i++) {
      output_names.push_back(info->output_names[i]);
    }
    indicator.setOutputNames(output_names);

    /* Push to result vector */
    result.push_back(indicator);

    /* Next indicator */
    ++info;
  }

  callback.call<int>("", result);
  return 0;
}

#include "nbind/nbind.h"

NBIND_CLASS(Indicator) {
  getset(getName, setName);
  getset(getFullName, setFullName);
  getset(getType, setType);
  getset(getInputs, setInputs);
  getset(getOutputs, setOutputs);
  getset(getOptions, setOptions);
  getset(getInputNames, setInputNames);
  getset(getOptionNames, setOptionNames);
  getset(getOutputNames, setOutputNames);
}

NBIND_GLOBAL() {
  function(list_indicators);
  function(indicator);
}

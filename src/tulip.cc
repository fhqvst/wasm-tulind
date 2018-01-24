#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "nbind/nbind.h"
#include "../tulip/indicators.h"

std::vector<std::string> sma() {

  // Input data
  const double data_in[] = {5,8,12,11,9,8,7,10,11,13};

  // Length of input
  const int input_length = sizeof(data_in) / sizeof(double);

  // Options array
  const double options[] = {3};

  // Start amount
  const int start = ti_sma_start(options);

  // Length of output
  const int output_length = input_length - start;

  // Pointer to output data
  double *data_out = new double[output_length * sizeof(double)];

  // Prepare inputs and outputs
  const double *all_inputs[] = {data_in};
  double *all_outputs[] = {data_out};

  // Run SMA
  int error = ti_sma(input_length, all_inputs, options, all_outputs);
  assert(error == TI_OKAY);

  // Place result in string vector
  std::vector<std::string> result;
  for (int i = 0; i < output_length; i++) {
    std::string out = std::to_string(data_out[i]);
    result.push_back(out);
  }

  return result;
}

NBIND_GLOBAL() {
  function(sma);
}


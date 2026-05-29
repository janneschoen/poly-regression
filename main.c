#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_ARGS 4
const char reqArgs[NUM_ARGS][50] = {
    "path of input file",
    "path of response file",
    "path of output file",
    "degree of regression",
};

int main(int argc, char * argv[]){
    if(argc != NUM_ARGS + 1){
        printf("Error: expected the following arguments:\n");
        for(unsigned i = 0; i < NUM_ARGS; i++){
            printf("%u: %s\n", i, reqArgs[i]);
        }
        return 1;
    }

    char * input_file = argv[1];
    char * response_file = argv[2];
    unsigned degree = (unsigned)atoi(argv[4]);
    unsigned matrix_dimensions = degree + 1;


    // Count lines of file

    unsigned count_lines(char * file_name){
        unsigned number_of_lines = 0;
        double number;
        FILE * file = fopen(file_name, "r");
        while(fscanf(file, "%lf", &number) != EOF){
            number_of_lines ++;
        }
        return number_of_lines;
    }

    // Read double numbers of file into array

    void read_numbers(char * file_name, double * array, unsigned number){
        FILE * file = fopen(file_name, "r");
        for(unsigned i = 0; i < number; i++){
            fscanf(file, "%lf", &array[i]);
        }
    }

    unsigned number_of_inputs = count_lines(input_file);
    unsigned number_of_points = count_lines(response_file);

    double inputs[number_of_inputs];
    double responses[number_of_points];
    read_numbers(input_file, inputs, number_of_inputs);
    read_numbers(response_file, responses, number_of_points);

    printf("Number of observations: %u\n", number_of_points);
    printf("Inputs with unknown response: %u\n", number_of_inputs - number_of_points);


    // Create design / feature matrix

    double design_matrix[number_of_points][matrix_dimensions];
    for(unsigned i = 0; i < number_of_points; i++){
        for(unsigned j = 0; j < matrix_dimensions; j++){
            design_matrix[i][j] = pow(inputs[i], j);
        }
    }

    // Transpose design matrix

    double transpose[matrix_dimensions][number_of_points];
    for(unsigned i = 0; i < matrix_dimensions; i++){
        for(unsigned j = 0; j < number_of_points; j++){
            transpose[i][j] = design_matrix[j][i];
        }
    }

    // Calculate Gram matrix (X^T * X)
    
    double gram_matrix[matrix_dimensions][matrix_dimensions];
    for(unsigned i = 0; i < matrix_dimensions; i++){
        for(unsigned j = 0; j < matrix_dimensions; j++){
            double value = 0;
            for(unsigned k = 0; k < number_of_points; k++){
                value += transpose[i][k] * design_matrix[k][j];
            }
            gram_matrix[i][j] = value;
        }
    }


    // Invert the Gram matrix using Gaussian Elimination

    // Set up augmented matrix (covariance matrix + identity matrix)

    double augmented_matrix[matrix_dimensions][matrix_dimensions * 2];
    for(unsigned i = 0; i < matrix_dimensions; i++){
        for(unsigned j = 0; j < matrix_dimensions * 2; j++){
            if(j < matrix_dimensions){
                augmented_matrix[i][j] = gram_matrix[i][j];
            } else if(i == j - matrix_dimensions){
                augmented_matrix[i][j] = 1;
            } else{
                augmented_matrix[i][j] = 0;
            }
        }
    }


    // Divide a line by its part of the diagonale

    void norm_line(unsigned i){
        double divisor = augmented_matrix[i][i];
        for(unsigned j = 0; j < matrix_dimensions * 2; j++){
            augmented_matrix[i][j] /= divisor;
        }
    }


    // Subtract line y times z from line x

    void subtract_linear_combination(unsigned x, unsigned y, double z){
        for(unsigned i = 0; i < matrix_dimensions * 2; i++){
            augmented_matrix[x][i] -= augmented_matrix[y][i] * z;
        }
    }

    void switch_lines(unsigned x, unsigned y){
        for(unsigned i = 0; i < matrix_dimensions * 2; i++){
            double value = augmented_matrix[x][i];
            augmented_matrix[x][i] = augmented_matrix[y][i];
            augmented_matrix[y][i] = value;
        }
    }

    for(unsigned i = 0; i < matrix_dimensions; i++){
        unsigned times_switched = 0;
        while(augmented_matrix[i][i] == 0){
            if(times_switched == matrix_dimensions || i+1 == matrix_dimensions){
                printf("Error: matrix not invertible.\n");
                return 1;
            }

            // if value at [i][i] is 0, switch lines
            // -> makes sure diagonale only has non-zero values

            switch_lines(i, i+1);
            times_switched++;
        }
        norm_line(i);

        for(unsigned j = 0; j < matrix_dimensions; j++){
            if(j != i){
                double factor = augmented_matrix[j][i];
                subtract_linear_combination(j, i, factor);
            }
        }
    }

    for(unsigned i = 0; i < matrix_dimensions; i++){
        for(unsigned j = 0; j < matrix_dimensions; j++){
            if((i == j && augmented_matrix[i][j] != 1) || (i != j && augmented_matrix[i][j] != 0)){
                printf("Error: matrix could not be inverted.\n");
                return 1;
            }
        }
    }


    // Read inverse from modified augmented matrix

    double inverse[matrix_dimensions][matrix_dimensions];
    for(unsigned i = 0; i < matrix_dimensions; i++){
        for(unsigned j = 0; j < matrix_dimensions; j++){
            inverse[i][j] = augmented_matrix[i][j+matrix_dimensions];
        }
    }


    // Calculate cross product vector (transpose of design matrix * response vector)

    double cross_product_vector[matrix_dimensions];
    for(unsigned i = 0; i < matrix_dimensions; i++){
        double value = 0;
        for(unsigned j = 0; j < number_of_points; j++){
            value += transpose[i][j] * responses[j];
        }
        cross_product_vector[i] = value;
    }


    // Calculate coefficients (inverse of Gram matrix * cross product vector)

    double coefficients[matrix_dimensions];
    for(unsigned i = 0; i < matrix_dimensions; i++){
        double value = 0;
        for(unsigned j = 0; j < matrix_dimensions; j++){
            value += inverse[i][j] * cross_product_vector[j];
        }
        coefficients[i] = value;
    }


    // Predict responses to inputs based on calculated function

    double predictions[number_of_inputs];
    for(unsigned i = 0; i < number_of_inputs; i++){
        predictions[i] = coefficients[0];
        for(unsigned j = 1; j < matrix_dimensions; j++){
            predictions[i] += coefficients[j] * pow(inputs[i], j);
        }
    }


    // Calculate mean squared error

    double squared_errors[number_of_points];
    for(unsigned i = 0; i < number_of_points; i++){
        squared_errors[i] = pow(predictions[i] - responses[i], 2);
    }
    double mean_squared_error = 0;
    for(unsigned i = 0; i < number_of_points; i++){
        mean_squared_error += squared_errors[i];
    }
    mean_squared_error /= number_of_points;

    printf("\nMSE: %.5f\n", mean_squared_error);
    printf("MSE Root: %.5f\n", pow(mean_squared_error, 0.5));

    printf("\nCoefficients:\n");
    printf("%lf", coefficients[0]);
    for(unsigned i = 1; i < matrix_dimensions; i++){
        printf(" + %lf x^%u", coefficients[i], i);
    }
    printf("\n");


    // Write predictions to file
    
    FILE * file = fopen(argv[3], "w");
    for(unsigned i = 0; i < number_of_inputs; i++){
        fprintf(file, "%lf\n", predictions[i]);
    }
    fclose(file);

    return 0;
}
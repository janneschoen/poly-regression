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

    char * inputFile = argv[1];
    char * responseFile = argv[2];
    unsigned degree = (unsigned)atoi(argv[4]);
    unsigned matDim = degree + 1;

    // Counting lines of file
    unsigned countLines(char * fileName){
        unsigned numLines = 0;
        double num;
        FILE * file = fopen(fileName, "r");
        while(fscanf(file, "%lf", &num) != EOF){
            numLines ++;
        }
        return numLines;
    }

    // Reading double numbers of file into array
    void readNumbers(char * fileName, double * array, unsigned num){
        FILE * file = fopen(fileName, "r");
        for(unsigned i = 0; i < num; i++){
            fscanf(file, "%lf", &array[i]);
        }
    }

    unsigned numInputs = countLines(inputFile);
    unsigned numPoints = countLines(responseFile);

    double inputs[numInputs];
    double responses[numPoints];
    readNumbers(inputFile, inputs, numInputs);
    readNumbers(responseFile, responses, numPoints);

    printf("Number of observations: %u\n", numPoints);
    printf("Inputs with unknown response: %u\n", numInputs - numPoints);


    // Creating design / feature matrix

    double designMat[numPoints][matDim];
    for(unsigned i = 0; i < numPoints; i++){
        for(unsigned j = 0; j < matDim; j++){
            designMat[i][j] = pow(inputs[i], j);
        }
    }

    // Transposing design matrix

    double transpose[matDim][numPoints];
    for(unsigned i = 0; i < matDim; i++){
        for(unsigned j = 0; j < numPoints; j++){
            transpose[i][j] = designMat[j][i];
        }
    }

    // Calculating Gram matrix (X^T * X)
    
    double gramMat[matDim][matDim];
    for(unsigned i = 0; i < matDim; i++){
        for(unsigned j = 0; j < matDim; j++){
            double value = 0;
            for(unsigned k = 0; k < numPoints; k++){
                value += transpose[i][k] * designMat[k][j];
            }
            gramMat[i][j] = value;
        }
    }

    // Inverting the Gram matrix using Gaussian Elimination

    // Setting up augmented matrix (covariance matrix + identity matrix)
    double augMat[matDim][matDim * 2];
    for(unsigned i = 0; i < matDim; i++){
        for(unsigned j = 0; j < matDim * 2; j++){
            if(j < matDim){
                augMat[i][j] = gramMat[i][j];
            } else if(i == j - matDim){
                augMat[i][j] = 1;
            } else{
                augMat[i][j] = 0;
            }
        }
    }

    // Dividing a line by its part of the diagonale
    void normLine(unsigned i){
        double divisor = augMat[i][i];
        for(unsigned j = 0; j < matDim * 2; j++){
            augMat[i][j] /= divisor;
        }
    }

    // Subtrating line y times z from line x
    void linCombSub(unsigned x, unsigned y, double z){
        for(unsigned i = 0; i < matDim * 2; i++){
            augMat[x][i] -= augMat[y][i] * z;
        }
    }

    void switchLines(unsigned x, unsigned y){
        for(unsigned i = 0; i < matDim * 2; i++){
            double value = augMat[x][i];
            augMat[x][i] = augMat[y][i];
            augMat[y][i] = value;
        }
    }

    for(unsigned i = 0; i < matDim; i++){
        unsigned timesSwitched = 0;
        while(augMat[i][i] == 0){
            if(timesSwitched == matDim || i+1 == matDim){
                printf("Error: matrix not invertible.\n");
                return 1;
            }
            // if value at [i][i] is 0, switch lines
            // -> makes sure diagonale only has non-zero values
            switchLines(i, i+1);
            timesSwitched++;
        }
        normLine(i);

        for(unsigned j = 0; j < matDim; j++){
            if(j != i){
                double factor = augMat[j][i];
                linCombSub(j, i, factor);
            }
        }
    }

    for(unsigned i = 0; i < matDim; i++){
        for(unsigned j = 0; j < matDim; j++){
            if((i == j && augMat[i][j] != 1) || (i != j && augMat[i][j] != 0)){
                printf("Error: matrix could not be inverted.\n");
                return 1;
            }
        }
    }

    // Reading inverse from modified augmented matrix

    double inverse[matDim][matDim];
    for(unsigned i = 0; i < matDim; i++){
        for(unsigned j = 0; j < matDim; j++){
            inverse[i][j] = augMat[i][j+matDim];
        }
    }

    // Calculating cross product vector (transpose of design matrix * response vector)
    double crossProductVec[matDim];
    for(unsigned i = 0; i < matDim; i++){
        double value = 0;
        for(unsigned j = 0; j < numPoints; j++){
            value += transpose[i][j] * responses[j];
        }
        crossProductVec[i] = value;
    }

    // Calculating coefficients (inverse of Gram matrix * cross product vector)
    double coefficients[matDim];
    for(unsigned i = 0; i < matDim; i++){
        double value = 0;
        for(unsigned j = 0; j < matDim; j++){
            value += inverse[i][j] * crossProductVec[j];
        }
        coefficients[i] = value;
    }

    // Predict responses to inputs based on calculated function

    double predictions[numInputs];
    for(unsigned i = 0; i < numInputs; i++){
        predictions[i] = coefficients[0];
        for(unsigned j = 1; j < matDim; j++){
            predictions[i] += coefficients[j] * pow(inputs[i], j);
        }
    }

    // Calculate mean squared error

    double squaredErrors[numPoints];
    for(unsigned i = 0; i < numPoints; i++){
        squaredErrors[i] = pow(predictions[i] - responses[i], 2);
    }
    double meanSquaredError = 0;
    for(unsigned i = 0; i < numPoints; i++){
        meanSquaredError += squaredErrors[i];
    }
    meanSquaredError /= numPoints;

    printf("\nMSE: %.5f\n", meanSquaredError);
    printf("MSE Root: %.5f\n", pow(meanSquaredError, 0.5));

    printf("\nCoefficients:\n");
    printf("%lf", coefficients[0]);
    for(unsigned i = 1; i < matDim; i++){
        printf(" + %lf x^%u", coefficients[i], i);
    }
    printf("\n");


    // Writing predictions to file
    FILE * file = fopen(argv[3], "w");
    for(unsigned i = 0; i < numInputs; i++){
        fprintf(file, "%lf\n", predictions[i]);
    }
    fclose(file);

    return 0;
}
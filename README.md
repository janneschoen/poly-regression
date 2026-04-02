# Poly-Regression
**Fast polynomial regression in C** — quick least-squares fitting for polynomials of any degree, with MSE evaluation and coefficient output.

## Features

- Pure C implementation (0 external libraries)
- Supports arbitrary polynomial degrees
- Includes mean squared error (MSE) calculation
- Simple and efficient

## Quick Start

### Prerequisites
- GCC (or any other C compiler)
- Make (optional)

### Installation
```bash
git clone https://github.com/janneschoen/poly-regression.git
cd poly-regression
make
```
## Usage
```bash
./polyreg <input file> <response file> <output file> <degree>
```
### Input Format
The program expects two text files, one of which contains the input values, and the other the responses.


### Output
- Polynomial coefficients
- Mean Squared Error (and square root of it)
- Predictions written to file

## Example Usage
```bash
./polyreg inputFile.txt responseFile.txt predictions.out 2
```
In this example, the coefficients of a quadratic function are calculated to try to match the relationship of the data from `inputFile.txt` (12 values) and `responseFile.txt` (10 values), and the predictions are saved in `predictions.out`.

Given observations:

<img height="300" alt="Observation plot" src="https://github.com/user-attachments/assets/9e516d11-135e-4538-ba17-cf772121f5b1" />

This is the estimated regression of the program:

<img height="300" alt="Regression and observation plot" src="https://github.com/user-attachments/assets/f84c9245-2b8c-491e-a6e2-cd5773bda74f" />

*Note: The regression function extends beyond the given observations since the input file contained more values than the response file.*

The program saved the predictions in `predictions.out` and generated the following output in the terminal:

```bash
Number of observations: 10
Inputs with unknown response: 2

MSE: 24.82061
MSE Root: 4.98203

Coefficients:
0.866667 + -1.806061 x^1 + 1.212121 x^2
```

## How?
The mathematical procedure used by the program is **Ordinary Least Squares (OLS)**.

The formula to calculate the coefficients: $β = (X^T X)^{-1} X^T y$

It consists of the following steps:

### 1. Vandermonde / Design Matrix - $X$
The first step is to create the design matrix, where each row represent one processed input.
When doing quadratic regression, the row corresponding to input `5` would look like this:
`1 5 25`
Each value is calculated by x^i, with i = [0,2] in the case of a quadratic function.

### 2. Gram Matrix - $X^T X$
To compute the Gram matrix, the first step is to calculate the transpose of the Vandermonde matrix.

The transpose is easily calculated by switching rows and columns.
By multiplying the transpose with the Vandermonde matrix, we obtain the Gram matrix.

Since $X$ has dimensions (observations x coefficients), and $X^T$ (coefficients x observations), the Gram matrix is square, and typically smaller: (coefficients x coefficients).

### 3. Inverting the Gram Matrix - $(X^T X)^{-1}$
The program uses Gaussian elimination to invert the Gram matrix, which is the trickiest part of the computation.
This works by transforming the Gram matrix into the identity matrix step by step, while applying each modification to a parallel identity matrix as well.

To turn the Gram matrix into the identity matrix, the diagonale has to consist of `1`s, while all other values are `0`.
The values of the diagonale are the values `matrix[i][i]` - where both coordinates are the same.

#### Switching
At the start of each iteration, if the diagonal value of the row currently edited is `0`, it is switched with the row below. If there is no row with a non-zero value at the necessary position, it indicates that some columns are linearly dependent on each other - so the matrix is not invertible.

#### Norming
To achieve the goal of a diagonale consisting of `1`s, a row is "normed" in the next step. So each element in the row is divided by the diagonal value `matrix[i][i]`. This results in the diagonal value being `1`.

#### Subtracting
The result of the previous steps should be a row with a `1` at the diagonal position `[i][i]`. By multiplying this row and subtracting it from another row, all values in column `i` can be set to `0`, except for the value at the diagonale.

If the matrix was not singular (rows or columns linearly dependent), the inverse should now have been successfully calculated.

### 4. Obtaining Coefficients - $β$
The final step to obtain the coefficients is to calculate the multiply the inverse of the Gram matrix by the product of the Vandermonde-Transpose and the response / target vector.

The vector consisting of the responses to the inputs has the same number of elements as the Vandermonde-Transpose has columns. So by multiplying them, a vector of a length that matches the number of coefficients is obtained.

To get the array of coefficients, the program now finally multiplies the inverse of the Gram matrix by this vector.

In the case of quadratic regression, coefficient array `[2, 4, 3]`, it is interpreted as the following function:
$f(x) = 2 + 4x + 3x²$

## Visualisation
The project repository contains a simple Python plotting script useful for visualising your observations and how well the regression fits them. It is easily executed:
`python plotting.py <input.txt> <response.txt> <predictions.out>`

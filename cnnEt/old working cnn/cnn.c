#include <stdio.h>
#include <math.h>
#include "cnn.h"

double max (size_t dim, double *x);
double sum (size_t dim, double *x);
void nv_exp (size_t dim, double *x, double m);


/**
* Implements a convolution layer
* @param feat_map:	# of resulting feature maps
* @param channel:	# of input channels
* @param in:		input tensor, size = in_x * in_y * channel
* @param w:			weights tensor, size = f_x * f_y * channel * feat_map
* @param b:			bias vector, size = feat_map
* @param out:		destination tensor
*/
void convolution (size_t channel, size_t in_x, size_t in_y, double in[channel][in_x][in_y], size_t feat_map, size_t f_x, size_t f_y,
	double w[feat_map][channel][f_x][f_y], double b[feat_map], size_t o_x, size_t o_y, double out[feat_map][o_x][o_y])
{
	double tmp;
	int i,j,k,l,s,t,m,n;

    //Convolution
	for (k = 0; k < feat_map; k++){
		for (i = 0; i < o_x; i++){
			for (j = 0; j < o_y; j++){	
				for (l = 0; l < channel; l++){			
					for (s = 0; i < f_x; i++){
						for (t = 0; j < f_y; j++){
							out[k][i][j] = b[k];					//Adding bias
						    m = i + s;								//Input width index
							n = j + t;								//Input height index
							tmp = w[k][l][s][t] * in[l][m][n];
							out[k][i][j] += tmp;
							out[k][i][j] = tanh(out[k][i][j]);		//Non linearity			
						}
					}
				}
			}
		}
	}

}

/**
* Implements a max pooling layer
* @param feat_map:	# of feature maps to be pooled
* @param f_x:		height of the pooling filter
* @param f_y:		width of the pooling filter
* @param in:		input tensor, size = in_x * in_y * feat_map
* @param out:		output tensor, size = o_x * o_y * feat_map
*/ 
void max_pooling (size_t feat_map, size_t in_x, size_t in_y, double in[feat_map][in_x][in_y], size_t o_x, size_t o_y,
				  double out[feat_map][o_x][o_y], size_t f_x, size_t f_y)
{
	int i,j,k,m,n,s,t;
	double max;
			
	for (k = 0; k < feat_map; k++){	
		for (i = 0; i < o_x; i++){
			for (j = 0; j < o_y; j++){			
				max = -HUGE_VAL;
				for (s = 0; s < f_x; s++){
					for (t = 0; t < f_y; t++){
						m = i*2;
						m += s;
						n = j*2;
						n += t;
						if (in[k][m][n] > max){
							max = in[k][m][n];
						}
					}
				}
				out[k][i][j] = max;
			}
		}
	}

}

/**
* Implements a layer of a simple neural network
* @param in: 	input vector, size = in_dim
* @param w:	 	weights matrix, size = in_dim * out_dim
* @param b:  	biases vector, size = out_dim
* @oaram out:	output vector, size = out_dim
*/
void linear (size_t in_dim, double in[in_dim], size_t out_dim, double out[out_dim], double w[in_dim][out_dim], double b[out_dim])
{
	int i, j;
	double tmp;

	for (i = 0; i < out_dim; i++){
		for (j = 0; j < in_dim; j++){
			out[i] = b[i];				//Adding bias
			tmp = in[j] * w[i][j];
			out[i] += tmp;			
		}
	}

}

/**
* Applies a non-linearity to the input vector
* @param v: input vector, size = dim
*/
void non_linearity (size_t dim, double * v)
{
	int i;
	
	for (i = 0; i < dim; i++){
		v[i] = tanh(v[i]);
	}

}

/**
* Applies the LogSoftMax operator to the output vector of the MLP.
* @param classes:	# of classes to be recognized
* @param in:		output vector of the MLP, size = classes
* @param r:			final output vector, size = classes
*/
void classify (size_t classes, double in[classes], double r[classes])
{
	int i;
	double s, m;

	m = max (classes, in);
	nv_exp(classes, in, m);
	s = sum (classes, in);
	for (i = 0; i < classes; i++){
		r[i] = in[i] / s;
		//r[i] = log(r[i]);
	}

}

/**
* Transforms the output of the convolutional part of the network to a monodimensional vector,
* which will be passed to the MLP at the end of the network.
* @param feat_map:	# of feature maps out of the last convolution
* @param in:		input tensor, size = x * y * feat_map
* @param out:		output vector, size = dim
*/
void reshape (size_t feat_map, size_t x, size_t y, size_t dim, double in[feat_map][x][y], double out[dim])
{
	int i,j,k,a,b;
	
	for (k = 0; k < feat_map; k++){
		for (i = 0; i < x; i++){
			for (j = 0; j < y; j++){
				a = k*x;
				a = a*y;
				b = i*x;
				b = b + j;
				b = a + b;
				out[b] = in[k][i][j];
			}
		}
	}
}

double max (size_t dim, double *x)
{
	int i;
	double m = -HUGE_VAL;
	
	for (i = 0; i < dim; i++){
		if (x[i] > m){
			m = x[i];
		}
	}

	return m;
}

double sum (size_t dim, double *x)
{
	int i;
	double s = 0;

	for (i = 0; i < dim; i++){
		s = s + x[i];
	}

	return s;
}

void nv_exp (size_t dim, double *x, double m)
{
	int i;
	double y;

	for (i = 0; i < dim; i++){
		y = x[i] /*- m*/;
		x[i] = exp(y);
	}
}

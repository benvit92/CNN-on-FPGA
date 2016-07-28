#ifndef __CNN_H__
#define __CNN_H__
#include "stdio.h"

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
				  double w[feat_map][channel][f_x][f_y], double b[feat_map], size_t o_x, size_t o_y, double out[feat_map][o_x][o_y]);

/**
* Implements a max pooling layer
* @param feat_map:	# of feature maps to be pooled
* @param f_x:		height of the pooling filter
* @param f_y:		width of the pooling filter
* @param in:		input tensor, size = in_x * in_y * feat_map
* @param out:		output tensor, size = o_x * o_y * feat_map
*/ 
void max_pooling (size_t feat_map, size_t in_x, size_t in_y, double in[feat_map][in_x][in_y], size_t o_x, size_t o_y,
				  double out[feat_map][o_x][o_y], size_t f_x, size_t f_y);

/**
* Implements a layer of a simple neural network
* @param in: 	input vector, size = in_dim
* @param w:	 	weights matrix, size = in_dim * out_dim
* @param b:  	biases vector, size = out_dim
* @oaram out:	output vector, size = out_dim
*/
void linear (size_t in_dim, double in[in_dim], size_t out_dim, double out[out_dim], double w[in_dim][out_dim], double b[out_dim]);

/**
* Applies a non-linearity to the input vector
* @param v: input vector, size = dim
*/
void non_linearity (size_t dim, double * v);

/**
* Applies the LogSoftMax operator to the output vector of the MLP.
* @param classes:	# of classes to be recognized
* @param in:		output vector of the MLP, size = classes
* @param r:			final output vector, size = classes
*/
void classify (size_t classes, double in[classes], double r[classes]);

/**
* Transforms the output of the convolutional part of the network to a monodimensional vector,
* which will be passed to the MLP at the end of the network.
* @param feat_map:	# of feature maps out of the last convolution
* @param in:		input tensor, size = x * y * feat_map
* @param out:		output vector, size = dim
*/
void reshape (size_t feat_map, size_t x, size_t y, size_t dim, double in[feat_map][x][y], double out[dim]);

#endif /* __CNN_H__ */

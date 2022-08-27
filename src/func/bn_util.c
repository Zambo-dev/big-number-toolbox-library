/**
 * @file bn_util.c
 * @brief Utility functions for handling big numbers
 * @author Zambo-dev
 * @date 22/12/2021
 * @copyright Zambo-dev - 2022 | licensed under MIT
 */

#include "../include/bntl.h"

/**
 * 	@brief Free number's memory
 *	@param [in,out] number Initialized number
 */
void bn_free(bn_t *number)
{
	if (number->num != NULL)
	{
		free(number->num);
		number->num = NULL;
	}
	number->size = 0;
}

/**
 *	@brief Revers number's bytes
 *	@param [in,out] number Initialized number
 */
void bn_rev(bn_t *number)
{
	if (number == NULL || number->num == NULL)
		return;

	ubyte *start = (ubyte *)number->num;
	ubyte *end = (ubyte *)number->num + number->size - 1;
	ubyte tmp;
	size_t size = number->size / 2;

	while (size > 0)
	{
		tmp = *start;
		*start = *end;
		*end = tmp;

		++start;
		--end;
		--size;
	}
}

/**
 * 	@brief Copy value from number to result reallocating result memory
 *	@param [in,out] result Initialized number
 *	@param [in] number Initialized number
 */
void bn_cpy(bn_t *result, bn_t *number)
{
	if (result == NULL || number == NULL || number->num == NULL)
		return;
	if (result->num == NULL)
		bn_init(result, number->size);
	else
	{
		ulong *tmp;
		if ((tmp = realloc(result->num, number->size)) == NULL)
			return;

		result->num = tmp;
		result->size = number->size;
	}

	size_t size = number->size;

	ulong *res = result->num;
	ulong *num = number->num;
	size /= sizeof(ulong);

	while (size > 0)
	{
		*res = *num;
		++res;
		++num;
		--size;
	}
}

/**
 * 	@brief Copy value from number to result not reallocating result memory
 *	@param [in,out] result Initialized number
 *	@param [in] number Initialized number
 * 	@note Result can either be bigger or smaller than number, if is bigger the spare is set to 0, if is smaller only the available bytes are set
 */
void bn_hcpy(bn_t *result, bn_t *number)
{
	if (result == NULL || number == NULL || number->num == NULL)
		return;
	if (result->num == NULL)
		bn_init(result, number->size);

	ubyte *res, *num;
	ulong tmp = 1;
	byte inc;

	if(*(ubyte *)&tmp == 1) 		/* Small endian */
	{
		res = (ubyte *)result->num;
		num = (ubyte *)number->num;
		inc = 1;
	}
	else							/* Big endian */
	{
		res = (ubyte *)result->num + result->size - 1;
		num = (ubyte *)number->num + number->size - 1;
		inc = -1;
	}

	size_t num_size = number->size;
	size_t res_size = result->size;

	while (num_size > 0 && res_size > 0)
	{
		*res = *num;
		res += inc;
		num += inc;
		--num_size;
		--res_size;
	}

	while (res_size > 0)
	{
		*res = 0;
		res += inc;
		--res_size;
	}
}

/**
 * 	@brief Copy n bytes of value from number to result
 *	@param [in,out] result Initialized number
 *	@param [in] number Initialized number
 *	@param [in] size Byte size
 */
void bn_ncpy(bn_t *result, bn_t *number, size_t size)
{
	if (result == NULL || number == NULL || number->num == NULL)
		return;
	if (result->num == NULL)
		bn_init(result, size);

	ubyte *res, *num;
	ulong tmp = 1;
	byte inc;

	if(*(ubyte *)&tmp == 1) 		/* Small endian */
	{
		res = (ubyte *)result->num;
		num = (ubyte *)number->num;
		inc = 1;
	}
	else							/* Big endian */
	{
		res = (ubyte *)result->num + result->size - 1;
		num = (ubyte *)number->num + number->size - 1;
		inc = -1;
	}

	size_t res_size = result->size;
	if(size > number->size)
		size = (number->size > res_size) ? res_size : number->size;

	while (size > 0)
	{
		*res = *num;
		res += inc;
		num += inc;
		--size;
		--res_size;
	}

	while (res_size > 0)
	{
		*res = 0;
		res += inc;
		--res_size;
	}

	return 0;
}

/**
 * 	@brief Extend a number by n bytes
 *	@param [in,out] result Initialized number
 *	@param [in] number Initialized number
 *	@param [in] size Byte size
 */
void bn_ext(bn_t *number, size_t bytes)
{
	if (number == NULL)
		number = (bn_t *)malloc(sizeof(bn_t));
	if (number->num == NULL)
		bn_init(number, bytes);
	else
	{
		bn_t tmp;
		bn_init(&tmp, number->size + bytes);
		bn_hcpy(&tmp, number);
		bn_cpy(number, &tmp);
		bn_free(&tmp);
	}
}

/**
 * 	@brief Shrink unused bytes in front of the value
 *	@param [in,out] number Initialized number
 */
void bn_srk(bn_t *number)
{
	if (number == NULL || number->num == NULL)
		return;

	ulong *num;
	ulong tmp = 1;
	byte add;
	size_t size, old_size;

	if(*(ubyte *)&tmp == 1) 		/* Small endian */
	{
		num = number->num;
		add = 1;
	}
	else							/* Big endian */
	{
		num = number->num + number->size - 1;
		add = -1;
	}

	old_size = size = number->size / sizeof(int);

	while (--size > 0)
	{
		if ((*num & INT_MAX) != 0)
			break;

		num += add;
	}

	if (size != old_size)
	{
		bn_t tmp;
		bn_init(&tmp, size);
		bn_ncpy(&tmp, number, size);
		bn_cpy(number, &tmp);
		bn_free(&tmp);
	}
}

/**
 * 	@brief Shift value left by one bit
 *	@param [in,out] number Initialized number
 */
void bn_sl(bn_t *number, ulong val)
{
	if (number == NULL || number->num == NULL)
		return;

	ulong *num_end, *newnum, *start, *end;
	ulong tmp = 1, shift;
	ubyte carry = 0;
	byte inc;
	size_t size;
	
	/* Alocate new number for the byte shifting */
	newnum = (val >= sizeof(ulong)*8) ? (ulong *)calloc(number->size/sizeof(ulong), sizeof(ulong)) : NULL;

	/* Start from the higher value */
	if(*(ubyte *)&tmp == 1) 		/* Little endian */
	{
		num_end = number->num + number->size/sizeof(ulong) - 1;
		start = (newnum != NULL) ? newnum : number->num;
		end = ((newnum!= NULL) ? newnum : number-> num) + number->size/sizeof(ulong) - 1;
		/* Endian dependent increment value */
		inc = 1;
	}
	else							/* Big endian */
	{
		num_end = number->num;
		start = ((newnum != NULL) ? newnum : number->num) + number->size/sizeof(ulong) - 1; 
		end = (newnum != NULL) ? newnum : number->num;
		/* Endian dependent increment value */
		inc = -1;
	}

	/* Full byte to be shifted */
	shift = val/(sizeof(ulong)*8);
	/* Rest bits to be shifted */
	val -= shift*(sizeof(ulong)*8);
	/* Bytes offset to be copyed */
	num_end -= inc*shift;

	while(shift > 0)
	{
		*end = *num_end;
		/* Decrement iterators */
		end -= inc;
		num_end -= inc;
		--shift;
	}

	size = number->size/sizeof(ulong);
	
	while (size > 0 && val > 0)
	{
		/* Store the carry for the next chunk */
		tmp = (*start & (INT_MAX << val)) >> val;
		/* Shift the chunk */
		*start <<= val;
		/* Insert carry bytes in the shifteed space */
		*start |= carry;
		/* Set carry for next round */
		carry = tmp;
		tmp = 0;
		/* Increment iterators */
		start += inc;
		--size;
	}
	
	if(newnum != NULL)
	{
		free(number->num);
		number->num = newnum;
	}
}

/**
 * 	@brief Shift value right by one bit
 *	@param [in,out] number Initialized number
 * 	@note Require initialized number
 */
void bn_sr(bn_t *number, ulong val)
{

}

/**
 * 	@brief Compare two numbers
 *	@param [in] number1 Initialized number
 *	@param [in] number2 Initialized number
 *	@param [in,out] result Single byte initialized
 * 	@note If they're equal result is set to 0, if number1 is greater, result is set to 1, if number2 is greater, result is set to 2
 */
void bn_comp(bn_t *number1, bn_t *number2, ubyte *result)
{
	if(number1 == NULL && number2 == NULL)
	{
		*result = 0;
		return;
	}
	else if(number1 == NULL)
	{
		*result = 1;
		return;
	}
	else if(number2 == NULL)
	{
		*result = 2;
		return;
	}

	size_t size1 = number1->size;
	size_t size2 = number2->size;

	ubyte *num1 = (ubyte *)number1->num;
	ubyte *num2 = (ubyte *)number2->num;

	while ((size1--) > size2 && *(num1++) == 0)
	{
	}
	size1++;
	while (size1 < (size2--) && *(num2++) == 0)
	{
	}
	size2++;

	if (size1 > size2)
	{
		*result = 1;
		return;
	}
	else if (size1 < size2)
	{
		*result = 2;
		return;
	}

	*result = 0;

	while (size1 > 0)
	{
		if (*num1 > *num2)
		{
			*result = 1;
			break;
		}
		else if (*num1 < *num2)
		{
			*result = 2;
			break;
		}

		num1++;
		num2++;
		size1--;
	}
}

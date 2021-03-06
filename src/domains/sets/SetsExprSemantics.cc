/*-
 * Copyright (C) 2010-2014, Centre National de la Recherche Scientifique,
 *                          Institut Polytechnique de Bordeaux,
 *                          Universite de Bordeaux.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <domains/concrete/ConcreteExprSemantics.hh>
#include <domains/sets/SetsExprSemantics.hh>

#include <list>

/*! \brief Compute all possible values */
static SetsValue
generic_unary_semantic(ConcreteValue(*op_sem)(ConcreteValue, int, int),
		       SetsValue sv, int offset, int size)
{
  if (sv.is_any())
    return SetsValue(Option<ConcreteValue>());

  std::list<ConcreteValue> possible_values = sv.get_values().getValue();
  SetsValue result(BV_DEFAULT_SIZE);  // \todo check size

  for (std::list<ConcreteValue>::const_iterator
	 v  = possible_values.begin();
       v != possible_values.end();
       v++)
    result.add_value(Option<ConcreteValue>(op_sem(*v, offset, size)));

  return result;
}

/*! \brief Compute all possible values */
static SetsValue
generic_binary_semantic(ConcreteValue(*op_sem)(ConcreteValue, ConcreteValue,
					       int, int),
                        SetsValue sv1, SetsValue sv2, int offset, int size)
{

  if (sv1.is_any() || sv2.is_any())
    return SetsValue(Option<ConcreteValue>());

  std::list<ConcreteValue> possible_values1 = sv1.get_values().getValue();
  std::list<ConcreteValue> possible_values2 = sv2.get_values().getValue();
  SetsValue result(BV_DEFAULT_SIZE);

  for (std::list<ConcreteValue>::const_iterator
	 v1  = possible_values1.begin();
       v1 != possible_values1.end();
       v1++)
    for (std::list<ConcreteValue>::const_iterator
	   v2  = possible_values2.begin();
         v2 != possible_values2.end();
         v2++)
      result.add_value(Option<ConcreteValue>(op_sem(*v1, *v2, offset, size)));

  return result;
}

static SetsValue
generic_ternary_semantic(ConcreteValue(*op_sem)(ConcreteValue, ConcreteValue,
						ConcreteValue, int, int),
			 SetsValue sv1, SetsValue sv2, SetsValue sv3,
			 int offset, int size)
{

  if (sv1.is_any() || sv2.is_any() || sv3.is_any())
    return SetsValue(Option<ConcreteValue>());

  std::list<ConcreteValue> possible_values1 = sv1.get_values().getValue();
  std::list<ConcreteValue> possible_values2 = sv2.get_values().getValue();
  std::list<ConcreteValue> possible_values3 = sv3.get_values().getValue();
  SetsValue result(BV_DEFAULT_SIZE);

  for (std::list<ConcreteValue>::const_iterator
	 v1  = possible_values1.begin();
       v1 != possible_values1.end();
       v1++)
    for (std::list<ConcreteValue>::const_iterator
	   v2  = possible_values2.begin();
         v2 != possible_values2.end();
         v2++)
      for (std::list<ConcreteValue>::const_iterator
	     v3  = possible_values3.begin();
	   v3 != possible_values3.end();
	   v3++)
	result.add_value(Option<ConcreteValue>(op_sem(*v1, *v2, *v3,
						      offset, size)));

  return result;
}

#define particular_case_equal(v,the_val,the_result)			\
  try {									\
    ConcreteValue _aux = v.extract_value().getValue();			\
    if (_aux.equals (ConcreteValue(_aux.get_size (), (word_t) the_val))) \
      return SetsValue(Option<ConcreteValue>(ConcreteValue(_aux.get_size (), \
							   (word_t) the_result))); \
  } catch (OptionNoValueExc &) {}

template<> SetsValue
SetsExprSemantics::BV_OP_ADD_eval(SetsValue v1, SetsValue v2,
				  int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_ADD_eval, v1, v2,
				 offset, size);
}

template<> SetsValue
SetsExprSemantics::BV_OP_SUB_eval(SetsValue v1, SetsValue v2, int offset,
				  int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_SUB_eval, v1, v2,
				 offset, size);
}

template<> SetsValue
SetsExprSemantics::BV_OP_MUL_U_eval(SetsValue v1, SetsValue v2, int offset,
				    int size)
{
  particular_case_equal(v1, 0, 0);
  particular_case_equal(v2, 0, 0);
  SetsValue result =
    generic_binary_semantic(ConcreteExprSemantics::BV_OP_MUL_U_eval, v1, v2,
			    offset, size);

  return result;
}

template<>
SetsValue SetsExprSemantics::BV_OP_MUL_S_eval(SetsValue v1, SetsValue v2,
					      int offset, int size)
{
  particular_case_equal(v1, 0, 0);
  particular_case_equal(v2, 0, 0);
  SetsValue result =
    generic_binary_semantic(ConcreteExprSemantics::BV_OP_MUL_S_eval, v1, v2,
			    offset, size);

  return result;
}

template<>
SetsValue SetsExprSemantics::BV_OP_DIV_U_eval(SetsValue v1, SetsValue v2,
					      int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_DIV_U_eval, v1, v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_DIV_S_eval(SetsValue v1, SetsValue v2,
					      int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_DIV_S_eval, v1, v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_POW_eval(SetsValue v1, SetsValue v2,
					    int offset, int size)
{
  particular_case_equal(v2, 0, 1);
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_POW_eval, v1, v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_AND_eval(SetsValue v1, SetsValue v2,
					    int offset, int size)
{
  particular_case_equal(v1, 0, 0);
  particular_case_equal(v2, 0, 0);
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_AND_eval, v1, v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_OR_eval(SetsValue v1, SetsValue v2,
					   int offset, int size)
{
  if ((!(v1.contains(ConcreteValue(v1.get_size (), (word_t) 0)))) ||
      (!(v2.contains(ConcreteValue(v2.get_size (), (word_t) 0)))))
    return SetsValue(Option<ConcreteValue>(ConcreteValue(v1.get_size (), 1)));
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_OR_eval, v1, v2,
				 offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_XOR_eval(SetsValue v1, SetsValue v2,
					    int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_XOR_eval, v1, v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_LSH_eval(SetsValue v1, SetsValue v2,
					    int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_LSH_eval, v1,
				 v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_RSH_U_eval(SetsValue v1, SetsValue v2,
					      int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_RSH_U_eval,
				 v1, v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_RSH_S_eval(SetsValue v1, SetsValue v2,
					      int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_RSH_S_eval,
				 v1, v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_ROR_eval(SetsValue v1, SetsValue v2,
					    int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_ROR_eval, v1,
				 v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_ROL_eval(SetsValue v1, SetsValue v2,
					    int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_ROL_eval, v1,
				 v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_EQ_eval(SetsValue v1, SetsValue v2,
					   int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_EQ_eval, v1,
				 v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_NEQ_eval(SetsValue v1, SetsValue v2,
					    int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_NEQ_eval, v1,
				 v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_LEQ_S_eval(SetsValue v1, SetsValue v2,
					      int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_LEQ_S_eval,
				 v1, v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_LT_S_eval(SetsValue v1, SetsValue v2,
					     int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_LT_S_eval, v1, v2,
				 offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_LEQ_U_eval(SetsValue v1, SetsValue v2,
					      int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_LEQ_U_eval, v1,
				 v2, offset, size);
}

template<>
SetsValue SetsExprSemantics::BV_OP_LT_U_eval(SetsValue v1, SetsValue v2,
					     int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_LT_U_eval, v1,
				 v2, offset, size);
}

template<> SetsValue
SetsExprSemantics::BV_OP_EXTEND_U_eval(SetsValue v1, SetsValue v2,
				       int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_EXTEND_U_eval,
				 v1, v2, offset, size);
}

template<> SetsValue
SetsExprSemantics::BV_OP_EXTEND_S_eval(SetsValue v1, SetsValue v2,
				       int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_EXTEND_S_eval,
				 v1, v2, offset, size);
}

template<> SetsValue
SetsExprSemantics::BV_OP_MODULO_eval(SetsValue v1, SetsValue v2,
				     int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_MODULO_eval, v1,
				 v2, offset, size);
}

template<> SetsValue
SetsExprSemantics::BV_OP_CONCAT_eval(SetsValue v1, SetsValue v2,
				     int offset, int size)
{
  return generic_binary_semantic(ConcreteExprSemantics::BV_OP_CONCAT_eval, v1,
				 v2, offset, size);
}

template<> SetsValue
SetsExprSemantics::BV_OP_EXTRACT_eval(SetsValue v1, SetsValue v2, SetsValue v3,
				      int offset, int size)
{
  return generic_ternary_semantic(ConcreteExprSemantics::BV_OP_EXTRACT_eval,
				  v1, v2, v3, offset, size);
}

template<> SetsValue
SetsExprSemantics::BV_OP_NEG_eval(SetsValue v, int offset, int size)
{
  return generic_unary_semantic(ConcreteExprSemantics::BV_OP_NEG_eval, v,
				offset, size);
}

template<> SetsValue
SetsExprSemantics::BV_OP_NOT_eval(SetsValue v, int offset, int size)
{
  return generic_unary_semantic(ConcreteExprSemantics::BV_OP_NOT_eval, v,
				offset, size);
}

template<> SetsValue
SetsExprSemantics::embed_eval(SetsValue v1, SetsValue v2, int off) {
  return expr_semantics_embed_eval<SetsValue, SetsExprSemantics>(v1, v2, off);
}

template<> SetsValue
SetsExprSemantics::extract_eval(SetsValue v,  int off, int size) {
  return expr_semantics_extract_eval<SetsValue,
				     SetsExprSemantics>(v, off, size);
}

#undef particular_case_equal

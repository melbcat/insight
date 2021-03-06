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

#include "x86_translate.hh"

using namespace std;

X86_TRANSLATE_2_OP(MOV)
{
  Expr *src = op1;
  LValue *dst = (LValue *) op2;

  if (src->get_bv_size () > dst->get_bv_size ())
    Expr::extract_with_bit_vector_size_of (src, dst);
  else if (src->get_bv_size () < dst->get_bv_size ())
    Expr::extract_with_bit_vector_size_of ((Expr *&) dst, src);

  data.mc->add_assignment (data.start_ma, dst, src, data.next_ma);
}

X86_TRANSLATE_2_OP(MOVB)
{
  Expr::extract_bit_vector (op1, 0, 8);
  Expr::extract_bit_vector (op2, 0, 8);

  x86_translate<X86_TOKEN(MOV)> (data, op1, op2);
}

X86_TRANSLATE_2_OP(MOVW)
{
  Expr::extract_bit_vector (op2, 0, 16);

  x86_translate<X86_TOKEN(MOV)> (data, op1, op2);
}

X86_TRANSLATE_2_OP(MOVL)
{
  Expr::extract_bit_vector (op1, 0, 32);

  x86_translate<X86_TOKEN(MOV)> (data, op1, op2);
}

X86_TRANSLATE_2_OP(MOVQ)
{
  Expr::extract_bit_vector (op1, 0, 64);

  x86_translate<X86_TOKEN(MOV)> (data, op1, op2);
}

X86_TRANSLATE_2_OP(MOVABS)
{
  x86_translate<X86_TOKEN(MOV)> (data, op1, op2);
}

X86_TRANSLATE_2_OP(MOVBE)
{
  Expr *dst = op2;
  Expr *src = op1;
  Expr *dstbytes[4] = { NULL, NULL, NULL, NULL };
  MicrocodeAddress from (data.start_ma);
  int operand_size, nb_bytes;

  if (dst->is_MemCell ())
    {
      operand_size = src->get_bv_size ();
      nb_bytes = operand_size / 8;
      Expr *addr = dynamic_cast<MemCell *> (dst)->get_addr ();

      for (int i = 0; i < nb_bytes; i++)
	{
	  if (i == 0)
	    dstbytes[i] = MemCell::create (addr->ref (), 0, 8);
	  else
	    {
	      Expr *na =
		BinaryApp::create (BV_OP_ADD, addr->ref (),
				   Constant::create (i, 0,
						     addr->get_bv_size ()),
				   0, addr->get_bv_size ());

	      dstbytes[i] = MemCell::create (na, 0, 8);
	    }
	}
    }
  else
    {
      operand_size = dst->get_bv_size ();
      nb_bytes = operand_size / 8;

      for (int i = 0; i < nb_bytes; i ++)
	dstbytes[i] = dst->extract_bit_vector (8 * i, 8);
      Expr *tmp = src->extract_bit_vector (0, operand_size);
      src->deref ();
      src = tmp;
    }

  Expr *temp = data.get_tmp_register (TMPREG(0), operand_size);
  data.mc->add_assignment (from, (LValue *) temp->ref (), src->ref ());
  for (int i = 0; i < nb_bytes; i++)
    {
      MicrocodeAddress *next = (i == nb_bytes - 1) ? &data.next_ma : NULL;
      Expr *srcbyte =
	temp->extract_bit_vector (temp->get_bv_size () - 8 * (i + 1), 8);
      data.mc->add_assignment (from, (LValue *) dstbytes[i], srcbyte, next);
    }

  src->deref ();
  dst->deref ();
  temp->deref ();
}

static void
s_mov_on_cc (MicrocodeAddress from, x86::parser_data &data,
	     Expr *op1, Expr *op2, Expr *cond, MicrocodeAddress to)
{
  Expr *src = op1;
  LValue *dst = (LValue *) op2;

  if (src->get_bv_size () > dst->get_bv_size ())
    Expr::extract_with_bit_vector_size_of (src, dst);
  else if (src->get_bv_size () < dst->get_bv_size ())
    Expr::extract_with_bit_vector_size_of ((Expr *&) dst, src);

  data.mc->add_skip (from, to,
		     UnaryApp::create (BV_OP_NOT, cond, 0,
				       cond->get_bv_size ()));
  data.mc->add_skip (from, from + 1, cond->ref());
  from++;
  data.mc->add_assignment (from, dst, src, to);
}

#define X86_CC(cc, f) \
X86_TRANSLATE_2_OP (CMOV ## cc) \
{ s_mov_on_cc (data.start_ma, data, op1, op2, \
	       data.condition_codes[x86::parser_data::X86_CC_ ## cc]->ref (), \
	       data.next_ma); }

#include "x86_cc.def"
#undef X86_CC

X86_TRANSLATE_2_OP (CMOVC)
{
  s_mov_on_cc (data.start_ma, data, op1, op2,
	       data.condition_codes[x86::parser_data::X86_CC_B]->ref (),
	       data.next_ma);
}

X86_TRANSLATE_2_OP (CMOVNC)
{
  s_mov_on_cc (data.start_ma, data, op1, op2,
	       data.condition_codes[x86::parser_data::X86_CC_NB]->ref (),
	       data.next_ma);
}

static void
s_movx (x86::parser_data &data, Expr *op1, Expr *op2, BinaryOp op, int size)
{
  Expr *val = BinaryApp::createExtend (op, op1, size);
  assert (op2->get_bv_size () == size);

  x86_translate<X86_TOKEN(MOV)> (data, val, op2);

}

static void
s_movs (x86::parser_data &data, Expr *op1, Expr *op2, int size)
{
  s_movx (data, op1, op2, BV_OP_EXTEND_S, size);
}

static void
s_movz (x86::parser_data &data, Expr *op1, Expr *op2, int size)
{
  s_movx (data, op1, op2, BV_OP_EXTEND_U, size);
}

X86_TRANSLATE_2_OP(MOVSBW)
{
  Expr::extract_bit_vector (op1, 0, 8);
  s_movs (data, op1, op2, 16);
}

X86_TRANSLATE_2_OP(MOVSBL)
{
  Expr::extract_bit_vector (op1, 0, 8);
  s_movs (data, op1, op2, 32);
}

X86_TRANSLATE_2_OP(MOVSWL)
{
  Expr::extract_bit_vector (op1, 0, 16);
  s_movs (data, op1, op2, 32);
}

X86_TRANSLATE_2_OP(MOVSLQ)
{
  Expr::extract_bit_vector (op1, 0, 32);
  s_movs (data, op1, op2, 64);
}

X86_TRANSLATE_2_OP(MOVZBW)
{
  Expr::extract_bit_vector (op1, 0, 8);
  s_movz (data, op1, op2, 16);
}

X86_TRANSLATE_2_OP(MOVZBL)
{
  Expr::extract_bit_vector (op1, 0, 8);
  s_movz (data, op1, op2, 32);
}

X86_TRANSLATE_2_OP(MOVZWL)
{
  Expr::extract_bit_vector (op1, 0, 16);
  s_movz (data, op1, op2, 32);
}


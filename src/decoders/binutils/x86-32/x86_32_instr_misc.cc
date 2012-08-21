/*-
 * Copyright (C) 2010-2012, Centre National de la Recherche Scientifique,
 *                          Institut Polytechnique de Bordeaux,
 *                          Universite Bordeaux 1.
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

#include "x86_32_translation_functions.hh"

using namespace std;

X86_32_TRANSLATE_2_OP(BOUND)
{
  Expr *index = op1;
  Expr::extract_with_bit_vector_of (op2, op1);

  MemCell *min = dynamic_cast<MemCell *>(op2->ref ());

  assert (min != NULL);

  Expr *max_addr = 
    BinaryApp::create (BV_OP_ADD, min->get_addr ()->ref (),
		       op1->get_bv_size () / 8,
		       0, min->get_addr ()->get_bv_size ());

  Expr *max = MemCell::create (max_addr, 0, index->get_bv_size ());

  data.mc->add_skip (data.start_ma, data.next_ma,
		     BinaryApp::create (BV_OP_AND,
					BinaryApp::create (BV_OP_LEQ_S,
							   min,
							   index->ref (), 
							   0, 1),
					BinaryApp::create (BV_OP_LEQ_S,
							   index->ref (), 
							   max,
							   0, 1),
					0, 1));
  op1->deref ();
  op2->deref ();
}

X86_32_TRANSLATE_1_OP(BSWAP)
{
  assert (op1->get_bv_offset () == 0);
  assert (op1->get_bv_size () == 32);
  assert (op1->is_LValue ());

  MicrocodeAddress from (data.start_ma);
  LValue *temp = data.get_tmp_register (TMPREG(0), op1->get_bv_size ());

  data.mc->add_assignment (from, 
			   (LValue *) temp->ref (), 
			   op1->ref ());
  data.mc->add_assignment (from, 
			   (LValue *) op1->extract_bit_vector (0, 8),
			   temp->extract_bit_vector (24, 8));
  data.mc->add_assignment (from, 
			   (LValue *) op1->extract_bit_vector (8, 8),
			   temp->extract_bit_vector (16, 8));
  data.mc->add_assignment (from, 
			   (LValue *) op1->extract_bit_vector (16, 8),
			   temp->extract_bit_vector (8, 8));
  data.mc->add_assignment (from, 
			   (LValue *) op1->extract_bit_vector (24, 8),
			   temp->extract_bit_vector (0, 8),
			   data.next_ma);
  temp->deref ();
  op1->deref ();
}

X86_32_TRANSLATE_0_OP(CBW)
{
  data.mc->add_assignment (data.start_ma,
			   data.get_register ("ax"),
			   BinaryApp::create (BV_OP_EXTEND_S, 
					      data.get_register ("al"),
					      Constant::create (16)),
			   data.next_ma);
}

X86_32_TRANSLATE_0_OP(CBTW)
{
  x86_32_translate<X86_32_TOKEN (CBW)> (data);
}

X86_32_TRANSLATE_0_OP(CLC)
{
  x86_32_reset_CF (data.start_ma, data, NULL, &data.next_ma);
}

X86_32_TRANSLATE_0_OP(CLD)
{
  MicrocodeAddress from (data.start_ma);

  x86_32_reset_flag (from, data, "df", &data.next_ma);
}


X86_32_TRANSLATE_1_OP(CLFLUSH)
{
  log::warning << "CLFLUSH translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
  op1->deref ();
}

X86_32_TRANSLATE_0_OP(CLI)
{
  log::warning << "CLI translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
}


X86_32_TRANSLATE_0_OP(CLTS)
{
  log::warning << "CLTS translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
}

void
x86_32_cmpgen (MicrocodeAddress &from, x86_32::parser_data &data, 
	       Expr *op1, Expr *op2, MicrocodeAddress *to)
{
  Expr *src = op1;
  Expr *dst = op2;

  x86_32_set_operands_size ((Expr *&) dst, src);

  LValue *tmpr0 = data.get_tmp_register (TMPREG(0), dst->get_bv_size () + 1);

  data.mc->add_assignment (from, (LValue *) tmpr0->ref (), 
			   BinaryApp::create (BV_OP_SUB, dst->ref (), src, 0, 
					      tmpr0->get_bv_size ()));
  x86_32_assign_CF (from, data, 
		    tmpr0->extract_bit_vector (dst->get_bv_size (), 1), 
		    NULL);

  Expr *tmp[3];
  
  tmp[0] = dst->extract_bit_vector (dst->get_bv_size () - 1, 1);
  tmp[1] = src->extract_bit_vector (src->get_bv_size () - 1, 1);
  tmp[2] = tmpr0->extract_bit_vector (dst->get_bv_size () - 1, 1);

  tmp[1] = BinaryApp::create (BV_OP_XOR, tmp[0], tmp[1], 0, 1); 
  tmp[0] = BinaryApp::create (BV_OP_XOR, tmp[0]->ref (), tmp[2], 0, 1); 
  
  tmp[2] = BinaryApp::create (BV_OP_AND, tmp[0], tmp[1], 0, 1);

  x86_32_assign_OF (from, data, tmp[2]);

  dst->deref ();

  dst = (LValue *) tmpr0->extract_bit_vector (0, dst->get_bv_size ());

  tmpr0->deref ();  

  x86_32_compute_SF (from, data, dst);
  x86_32_compute_ZF (from, data, dst);
  x86_32_compute_AF (from, data, dst);
  x86_32_compute_PF (from, data, dst, to);
  dst->deref ();
}

X86_32_TRANSLATE_2_OP(CMP)
{
  MicrocodeAddress from = data.start_ma;
  x86_32_cmpgen (from, data, op1, op2, &data.next_ma);
}

X86_32_TRANSLATE_2_OP(CMPB)
{
  if (op1->get_bv_size () != 8)
    Expr::extract_bit_vector (op1, 0, 8);
  if (op2->get_bv_size () != 8)
    Expr::extract_bit_vector (op2, 0, 8);
  x86_32_translate<X86_32_TOKEN(CMP)> (data, op1, op2);
}

X86_32_TRANSLATE_2_OP(CMPW)
{
  if (op1->get_bv_size () != 16)
    Expr::extract_bit_vector (op1, 0, 16);
  if (op2->get_bv_size () != 16)
    Expr::extract_bit_vector (op2, 0, 16);
  x86_32_translate<X86_32_TOKEN(CMP)> (data, op1, op2);
}

X86_32_TRANSLATE_2_OP(CMPL)
{
  x86_32_translate<X86_32_TOKEN(CMP)> (data, op1, op2);
}

X86_32_TRANSLATE_2_OP(CMPXCHG)
{
  Expr *dst = op2;
  Expr *src = op1;
  Expr *eax = data.get_register ("eax");

  Expr::extract_bit_vector (eax, 0, op2->get_bv_size ());

  MicrocodeAddress from (data.start_ma + 1);
  MicrocodeAddress ifaddr (from);
  x86_32_assign_ZF (from, data, Constant::one (1));
  data.mc->add_assignment (from, (LValue *) dst->ref (), src->ref (),
			   data.next_ma);
  from++;
  MicrocodeAddress elseaddr (from);
  x86_32_reset_ZF (from, data);
  data.mc->add_assignment (from, (LValue *) eax->ref (), dst->ref (), 
			   data.next_ma);
  
  Expr *cond = BinaryApp::create (BV_OP_EQ, eax->ref (), dst->ref (), 0, 1);
  data.mc->add_skip (data.start_ma, elseaddr,
		     UnaryApp::create (BV_OP_NOT, cond->ref (), 0, 1));
  data.mc->add_skip (data.start_ma, ifaddr, cond->ref ());

  cond->deref ();
  dst->deref ();
  src->deref ();
  eax->deref ();
}

X86_32_TRANSLATE_0_OP(CMC)
{
  MicrocodeAddress from = data.start_ma;

  x86_32_assign_CF (from, data, 
		    UnaryApp::create (BV_OP_NOT, data.get_flag ("cf"), 0, 1), 
		    &data.next_ma);
}

X86_32_TRANSLATE_0_OP(CWDE)
{
  data.mc->add_assignment (data.start_ma,
			   data.get_register ("eax"),
			   BinaryApp::create (BV_OP_EXTEND_S, 
					      data.get_register ("ax"),
					      Constant::create (32)),
			   data.next_ma);
}

X86_32_TRANSLATE_0_OP(CWTL)
{
  x86_32_translate<X86_32_TOKEN (CWDE)> (data);
}


X86_32_TRANSLATE_0_OP(CPUID)
{
  log:: warning << "CPUID translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
}

X86_32_TRANSLATE_0_OP(CWD)
{
  data.mc->add_assignment (data.start_ma,
			   data.get_register ("dx"),
			   BinaryApp::create (BV_OP_EXTEND_S, 
					      data.get_register ("ax"),
					      Constant::create (32), 16, 16),
			   data.next_ma);  
}

X86_32_TRANSLATE_0_OP(CDQ)
{
  data.mc->add_assignment (data.start_ma,
			   data.get_register ("edx"),
			   BinaryApp::create (BV_OP_EXTEND_S, 
					      data.get_register ("eax"),
					      Constant::create (64), 32, 32),
			   data.next_ma);
}

X86_32_TRANSLATE_0_OP(HLT)
{
  data.mc->add_skip (data.start_ma, data.start_ma, Constant::zero (1));
}


X86_32_TRANSLATE_0_OP(INVD)
{
  log:: warning << "RDSTC translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
}

X86_32_TRANSLATE_1_OP(INVLPG)
{
  log:: warning << "RDSTC translated in INVLPG" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
  op1->deref ();
}

X86_32_TRANSLATE_0_OP(MWAIT)
{
  log:: warning << "MWAIT translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
}

X86_32_TRANSLATE_0_OP (NOP)
{
  data.mc->add_skip (data.start_ma, data.next_ma);
}

X86_32_TRANSLATE_1_OP(NOP)
{
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
  op1->deref ();
}

X86_32_TRANSLATE_1_OP(NOPW)
{
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
  op1->deref ();
}

X86_32_TRANSLATE_1_OP(NOPL)
{
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
  op1->deref ();
}

X86_32_TRANSLATE_0_OP(PAUSE)
{
  log:: warning << "PAUSE translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
}

/*
X86_32_TRANSLATE_0_OP (RDTSC)
{
  log:: warning << "RDSTC translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
}
*/

X86_32_TRANSLATE_1_OP(PREFETCHT0)
{
  log:: warning << "PREFETCHT0 translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
  op1->deref ();
}

X86_32_TRANSLATE_1_OP(PREFETCHT1)
{
  log:: warning << "PREFETCHT1 translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
  op1->deref ();
}

X86_32_TRANSLATE_1_OP(PREFETCHT2)
{
  log:: warning << "PREFETCHT2 translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
  op1->deref ();
}

X86_32_TRANSLATE_1_OP(PREFETCHNTA)
{
  log:: warning << "PREFETCHTNTA translated in NOP" << endl;
  x86_32_translate<X86_32_TOKEN (NOP)> (data);
  op1->deref ();
}

X86_32_TRANSLATE_2_OP(POPCNT)
{
  Expr *dst = op2;
  Expr *src = op1;
  const char *reset[] = { "of", "sf", "af", "cf", "pf", NULL };
  MicrocodeAddress from (data.start_ma);

  if (src->get_bv_size () > dst->get_bv_size ())
    Expr::extract_bit_vector (src, 0, dst->get_bv_size ());

  Expr *c = 
    BinaryApp::create (BV_OP_EXTEND_U, 
		       src->extract_bit_vector (0, 1),
		       Constant::create (dst->get_bv_size (), 
					 0, BV_DEFAULT_SIZE),
		       0, dst->get_bv_size ());

  data.mc->add_assignment (from, (LValue *) dst->ref (), c);
  for (int i = 1; i < dst->get_bv_size (); i++)
    {
      Expr *aux = 
	BinaryApp::create (BV_OP_EXTEND_U, 
			   src->extract_bit_vector (i, 1),
			   Constant::create (dst->get_bv_size (), 
					     0, BV_DEFAULT_SIZE),
			   0, dst->get_bv_size ());
      data.mc->add_assignment (from, (LValue *) dst->ref (), 
			       BinaryApp::create (BV_OP_ADD, dst->ref (), 
						  aux, 0, dst->get_bv_size ()));
    }


  
  x86_32_reset_flags (from, data, reset);
  x86_32_assign_ZF (from, data, 
		    BinaryApp::create (BV_OP_EQ, src->ref (), 
				       Constant::zero (src->get_bv_size ()), 
				       0, 1), 
		    &data.next_ma);
  dst->deref ();
  src->deref ();
}

X86_32_TRANSLATE_2_OP(XCHG)
{
  MicrocodeAddress from (data.start_ma);
  
  if (op1->is_MemCell ())
    Expr::extract_bit_vector (op1, 0, op2->get_bv_size ());
  else if (op2->is_MemCell ())
    Expr::extract_bit_vector (op2, 0, op1->get_bv_size ());

  LValue *temp = data.get_tmp_register (TMPREG(0), op1->get_bv_size ());
  data.mc->add_assignment (from, temp, op1->ref ());
  data.mc->add_assignment (from, (LValue *) op1, op2->ref ());
  data.mc->add_assignment (from, (LValue *) op2, temp->ref (), data.next_ma);
}

static void
s_set_cc (x86_32::parser_data &data, Expr *cond, Expr *dst)
{
  if (dst->get_bv_size () != 8)
    Expr::extract_bit_vector (dst, 0, 8);
  x86_32_if_then_else (data.start_ma, data, cond, 
		       data.start_ma + 1, data.start_ma + 2);
  data.mc->add_assignment (data.start_ma + 1, (LValue *) dst->ref (),
			   Constant::one (8), data.next_ma);
  data.mc->add_assignment (data.start_ma + 1, (LValue *) dst->ref (),
			   Constant::zero (8), data.next_ma);
  dst->deref ();
}

#define X86_32_CC(id, form) \
X86_32_TRANSLATE_1_OP(SET ## id)		\
{ \
  Expr *cond = \
    data.condition_codes[x86_32::parser_data::X86_32_CC_ ## id]->ref (); \
  s_set_cc (data, cond, op1); \
}
#include "x86_32_cc.def" 
#undef  X86_32_CC

X86_32_TRANSLATE_1_OP (SETC)
{
  x86_32_translate<X86_32_TOKEN(SETB)> (data, op1);
}

X86_32_TRANSLATE_1_OP (SETNC)
{
  x86_32_translate<X86_32_TOKEN(SETAE)> (data, op1);
}

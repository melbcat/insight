// The insight library and accompanying tools are distributed under a
// 2-clause "BSD license", as below.

// Copyright (c) 2010-2012, Centre National de la Recherche Scientifique,
//                          Institut Polytechnique de Bordeaux,
//                          Universite Bordeaux 1.

// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:

// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <kernel/Expressions.hh>
#include "BottomUpApplyVisitor.hh"

BottomUpApplyVisitor::BottomUpApplyVisitor() : FormulaVisitor () 
{ 
}

BottomUpApplyVisitor::~BottomUpApplyVisitor ()
{
}

void 
BottomUpApplyVisitor::visit (Constant *c)
{
  pre (c);
  apply (c);
}

void 
BottomUpApplyVisitor::visit (Variable *v)
{
  pre (v);
  apply (v);
}

void 
BottomUpApplyVisitor::visit (UnaryApp *ua)
{
  pre (ua);
  ua->get_arg1 ()->acceptVisitor (this);
  apply (ua);  
}
 
void 
BottomUpApplyVisitor::visit (BinaryApp *ba)
{
  pre (ba);
  ba->get_arg1 ()->acceptVisitor (this);
  ba->get_arg2 ()->acceptVisitor (this);
  apply (ba);  
}

void 
BottomUpApplyVisitor::visit (TernaryApp *ta)
{
  pre (ta);
  ta->get_arg1 ()->acceptVisitor (this);
  ta->get_arg2 ()->acceptVisitor (this);
  ta->get_arg3 ()->acceptVisitor (this);
  apply (ta);  
}

void 
BottomUpApplyVisitor::visit (MemCell *mc)
{
  pre (mc);
  mc->get_addr ()->acceptVisitor (this);
  apply (mc);
}
 
void 
BottomUpApplyVisitor::visit (RegisterExpr *reg)
{
  pre (reg);
  apply (reg);
}
 
void 
BottomUpApplyVisitor::visit (BooleanConstantFormula *c)
{
  pre (c);
  apply (c);
}
 
void 
BottomUpApplyVisitor::visit (ConjunctiveFormula *F)
{
  visit ((NaryBooleanFormula *) F);
}
 
void 
BottomUpApplyVisitor::visit (DisjunctiveFormula *F)
{
  visit ((NaryBooleanFormula *) F);
}
 
void 
BottomUpApplyVisitor::visit (NegationFormula *F)
{
  visit ((NaryBooleanFormula *) F);
}
 
void 
BottomUpApplyVisitor::visit (QuantifiedFormula *F)
{
  visit ((NaryBooleanFormula *) F);
}

void 
BottomUpApplyVisitor::pre (Formula *)
{
}

void 
BottomUpApplyVisitor::visit (NaryBooleanFormula *F) 
{
  pre (F);

  NaryBooleanFormula::Operands new_ops;
  NaryBooleanFormula::OperandsConstIterator it = F->const_operands_begin ();
  NaryBooleanFormula::OperandsConstIterator end = F->const_operands_end ();

  for (; it != end; it++)
    (*it)->acceptVisitor (this);
  apply (F);
}

 
ConstBottomUpApplyVisitor::ConstBottomUpApplyVisitor() : ConstFormulaVisitor () 
{ 
}

ConstBottomUpApplyVisitor::~ConstBottomUpApplyVisitor ()
{
}

void 
ConstBottomUpApplyVisitor::visit (const Constant *c)
{
  pre (c);
  apply (c);
}

void 
ConstBottomUpApplyVisitor::visit (const Variable *v)
{
  pre (v);
  apply (v);
}

void 
ConstBottomUpApplyVisitor::visit (const UnaryApp *ua)
{
  pre (ua);
  ua->get_arg1 ()->acceptVisitor (this);
  apply (ua);  
}
 
void 
ConstBottomUpApplyVisitor::visit (const BinaryApp *ba)
{
  pre (ba);
  ba->get_arg1 ()->acceptVisitor (this);
  ba->get_arg2 ()->acceptVisitor (this);
  apply (ba);  
}

void 
ConstBottomUpApplyVisitor::visit (const TernaryApp *ta)
{
  pre (ta);
  ta->get_arg1 ()->acceptVisitor (this);
  ta->get_arg2 ()->acceptVisitor (this);
  ta->get_arg3 ()->acceptVisitor (this);
  apply (ta);  
}

void 
ConstBottomUpApplyVisitor::visit (const MemCell *mc)
{
  pre (mc);
  mc->get_addr ()->acceptVisitor (this);
  apply (mc);
}
 
void 
ConstBottomUpApplyVisitor::visit (const RegisterExpr *reg)
{
  pre (reg);
  apply (reg);
}
 
void 
ConstBottomUpApplyVisitor::visit (const BooleanConstantFormula *c)
{  
  pre (c);
  apply (c);
}
 

void 
ConstBottomUpApplyVisitor::visit (const ConjunctiveFormula *F)
{
  visit ((const NaryBooleanFormula *) F);
}
 
void 
ConstBottomUpApplyVisitor::visit (const DisjunctiveFormula *F)
{
  visit ((const NaryBooleanFormula *) F);
}
 
void 
ConstBottomUpApplyVisitor::visit (const NegationFormula *F)
{
  visit ((const NaryBooleanFormula *) F);
}
 
void 
ConstBottomUpApplyVisitor::visit (const QuantifiedFormula *F)
{
  visit ((const NaryBooleanFormula *) F);
}
 
void 
ConstBottomUpApplyVisitor::pre (const Formula *)
{
}

void 
ConstBottomUpApplyVisitor::visit (const NaryBooleanFormula *F) 
{
  pre (F);
  NaryBooleanFormula::Operands new_ops;
  NaryBooleanFormula::OperandsConstIterator it = F->const_operands_begin ();
  NaryBooleanFormula::OperandsConstIterator end = F->const_operands_end ();

  for (; it != end; it++)
    (*it)->acceptVisitor (this);
  apply (F);
}


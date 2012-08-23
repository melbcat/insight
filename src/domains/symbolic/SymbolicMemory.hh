/*
 * Copyright (c) 2010-2012, Centre National de la Recherche Scientifique,
 *                          Institut Polytechnique de Bordeaux,
 *                          Universite Bordeaux 1.
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SYMBOLICMEMORY_HH
# define SYMBOLICMEMORY_HH

#include <tr1/unordered_map>

# include <kernel/Memory.hh>
# include <kernel/RegisterMap.hh>
# include <domains/concrete/ConcreteMemory.hh>
# include <domains/symbolic/SymbolicValue.hh>

class SymbolicMemory 
  : public Memory<ConcreteAddress, SymbolicValue>,
    public RegisterMap<SymbolicValue>
{
  typedef std::tr1::unordered_map<address_t, Expr *> MemoryMap;

public:
  SymbolicMemory (const ConcreteMemory *base);

  virtual ~SymbolicMemory();

  virtual SymbolicValue 
  get(const ConcreteAddress &a, int size, Architecture::endianness_t e) const
    throw (UndefinedValueException);

  virtual void put (const ConcreteAddress &a, const SymbolicValue &v, 
		    Architecture::endianness_t e);

  virtual bool is_defined(const ConcreteAddress &a) const;

  virtual SymbolicMemory *clone ();

  using RegisterMap<SymbolicValue>::is_defined;
  using RegisterMap<SymbolicValue>::get;
  using RegisterMap<SymbolicValue>::put;

private:
  const ConcreteMemory *base;
  MemoryMap memory;
};

#endif /* ! SYMBOLICMEMORY_HH */

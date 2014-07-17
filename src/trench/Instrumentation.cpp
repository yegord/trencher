/*
 * ----------------------------------------------------------------------------
 * "THE JUICE-WARE LICENSE" (Revision 42):
 * <calin@cs.uni-kl.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a glass of juice in return.
 * ----------------------------------------------------------------------------
 */

#include "Instrumentation.h"
#include <map>

#include "Expression.h"
#include "Foreach.h"
#include "Instruction.h"
#include "Program.h"
#include "State.h"
#include "Transition.h"
#include <iostream>

namespace trench {

namespace {
  State* awrite(Program &prog, Thread *thread, State *from, const std::string link, const std::string name, const int count, Write* write) {
    const std::shared_ptr<Constant> one(prog.makeConstant(1));
    const std::shared_ptr<Register> adr_reg(prog.makeRegister(link + "_adr_" + std::to_string(count)));
    const std::shared_ptr<Register> val_reg(prog.makeRegister(link + "_val_" + std::to_string(count)));

    // buffer the write using the newly added registers
    State *toadr = thread->makeState("adr_" + std::to_string(count) + "_" + link + name);
    State *toval = thread->makeState("val_" + std::to_string(count) + "_" + link + name);
    thread->makeTransition(from,toadr,std::make_shared<Local>(adr_reg,write->address()));
    thread->makeTransition(toadr,toval,std::make_shared<Local>(val_reg,write->value())); 
    
    return toval;  
  }

  void flush(Program &prog, Thread *thread, State *from, State *to, const std::string link, const std::string name, const int count) {
    for (int i=1; i<=count; i++) {
      const std::shared_ptr<Constant> iter(prog.makeConstant(i));
      const std::shared_ptr<Register> adr_reg(prog.makeRegister(link + "_adr_" + std::to_string(i)));
      const std::shared_ptr<Register> val_reg(prog.makeRegister(link + "_val_" + std::to_string(i)));

      State *flush = NULL;
      if (i==count) flush = to; else flush = thread->makeState("flush_" + std::to_string(i) + "_" + link + "_" + name);
      thread->makeTransition(from, flush, std::make_shared<Write>(val_reg,adr_reg,Space())); 
      from = flush;
    }
  }
  
  State* aread(Program &prog, Thread *thread, State *from, State *to, const std::string link, const std::string name, const int count, Read* read) {
    const std::shared_ptr<Constant> one = prog.makeConstant(1);
    for (int i=count; i>=1; i--) {
      const std::shared_ptr<Constant> iter(prog.makeConstant(i));
      const std::shared_ptr<Register> areg(prog.makeRegister(link + "_adr_" + std::to_string(i)));
      const std::shared_ptr<Register> vreg(prog.makeRegister(link + "_val_" + std::to_string(i)));
      const std::shared_ptr<Condition> eq(new Condition(std::make_shared<BinaryOperator>(BinaryOperator::EQ,areg,read->address())));
      const std::shared_ptr<Condition> neq(new Condition(std::make_shared<BinaryOperator>(BinaryOperator::NEQ,areg,read->address())));

      State *buf = thread->makeState("buf_" + std::to_string(i) + "_" + link + name);
      State *next = thread->makeState("next_" + std::to_string(i) + "_" + link + name);
      thread->makeTransition(from, buf, eq); // read address matches areg value
      thread->makeTransition(from, next, neq);
      thread->makeTransition(buf, to, std::make_shared<Local>(read->reg(),vreg));
      from = next;   
    }
    thread->makeTransition(from, to, std::make_shared<Read>(read->reg(),read->address(),Space()));
    return to;
  }
}

void instrument(Program &program, const Attack *attack, const std::vector<std::size_t> lines, const std::map<std::size_t,Transition*> &line2t) {
  foreach (Thread *thread, attack->program().threads()) {
    /* copy the original program */
    Thread *resultThread = program.makeThread(thread->name());
    if (thread->initialState()) {
      resultThread->setInitialState(resultThread->makeState(thread->initialState()->name()));
    }
    if (thread->finalState()) {
      resultThread->setFinalState(resultThread->makeState(thread->finalState()->name()));
    }
    foreach (Transition *transition, thread->transitions()) {
      State *from = resultThread->makeState(transition->from()->name());
      State *to = resultThread->makeState(transition->to()->name());
      resultThread->makeTransition(from,to,transition->instruction());
    }
    /* instrument the attack witness */
    if (thread == attack->attacker()) {
      State *from = resultThread->makeState(attack->write()->from()->name());
      std::stringstream ss; ss << attack->attacker(); std::string link = ss.str();
      // std::cout << "Instrumenting attacker for thread " << attack->attacker()->name() << " i.e. PTR_" << ss.str() << " ... " << std::endl;

      int count = 0; // track how many writes were buffered so far
      foreach (std::size_t line, lines) {
        if (line2t.at(line) != NULL) { // if NULL this is the `__done' line, otherwise transition at line belongs to witness support 
          std::string name = line2t.at(line)->to()->name();
          if (name == "final") {
            // read instruction closes the hb cycle
            from = aread(program, resultThread, from, resultThread->makeState(link), link, "", count, attack->read()->instruction()->as<Read>()); 
            // flush the buffer
            State *to = resultThread->makeState(attack->read()->to()->name());
            flush(program, resultThread, from, to, link, attack->read()->to()->name(), count);           
          } else if (name.substr(0,3) == "att") {
            std::shared_ptr<Instruction> instruction = line2t.at(line)->instruction();
            if (instruction->mnemonic() == Instruction::ATOMIC) {
              Atomic *atomic = instruction->as<Atomic>();
              if (Write *write = (atomic->instructions().at(0))->as<Write>()) { // attacker write to buffer
                count++;
                from = awrite(program, resultThread, from, link, name.substr(3), count, write);
                // nondeterministically flush and return to normal control flow // NEEDED ?
                // State *to = resultThread->makeState(name.substr(4));
                // flush(program, resultThread, from, to, link, name.substr(4), count);
              } else if (Write *write = (atomic->instructions().at(2))->as<Write>()) { // FIRST attacker write to buffer
                count++;
                from = awrite(program, resultThread, from, link, name.substr(3), count, write);
                // nondeterministically flush and return to normal control flow: NOT NEEDED !
                // State *to = resultThread->makeState(name.substr(4));
                // flush(program, resultThread, from, to, link, name.substr(4), count, zero); 
              } else if (Read *read = (atomic->instructions().at(3))->as<Read>()) { // attacker read 
                State *last = resultThread->makeState("last_" + link + name.substr(3)); // insert count? 
                from = aread(program, resultThread, from, last, link, name.substr(3), count, read); 
                // flush and return to normal program flow // NEEDED ?
                // State *to = resultThread->makeState(name.substr(4));
                // flush(program, resultThread, from, to, link, name.substr(4), count);
              } 
            } else if (instruction->as<Local>() || instruction->as<Condition>() || instruction->as<Noop>()) {
              State *to = resultThread->makeState("lcn_" + std::to_string(count) + "_" + link + name.substr(3));
              resultThread->makeTransition(from,to,instruction); from = to; 
            } else { /* attacker doesn't execute fences nor locked instructios */
              assert(!"No fences or locked instructions.");
            }
          }
        }
      }
    }
  }
}

} // namespace trench

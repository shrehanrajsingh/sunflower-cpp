#include "arithmetic.hpp"

namespace sf
{
Arithmetic
Arithmetic::from_infix (Vec<AVBase *> &vals)
{
  /**
   * Infix to Postfix
   * Starting off with a simple example
   * 2 + 3 * 4 ---> Infix
   * 2 3 4 * + ---> Postfix
   *
   * I1:
   * |   |   |   |
   * |   |   |   |
   * |   |   |   |
   * | 2 |   |   |
   * I2:
   * |   |   |   |
   * |   |   |   |
   * |   |   |   |
   * | 2 |   | + |
   * I3:
   * |   |   |   |
   * |   |   |   |
   * | 2 |   |   |
   * | 3 |   | + |
   * I4:
   * |   |   |   |
   * |   |   |   |
   * | 2 |   | + |
   * | 3 |   | * | ---> Push to stack while precedence of last stack op is <=
   *                     precedence of current op
   * I5:
   * |   |   |   |
   * | 2 |   |   |
   * | 3 |   | + |
   * | 4 |   | * |
   * Pop all from op stack and push to val stack
   * |  2  |
   * |  3  |
   * |  4  |
   * |  *  |
   * |  +  |
   *
   * Another example
   * 2 * 3 + 4 ----> Infix
   * 2 3 * 4 + ----> Postfix
   *
   * I1:
   * |   |   |   |
   * |   |   |   |
   * |   |   |   |
   * | 2 |   |   |
   * I2:
   * |   |   |   |
   * |   |   |   |
   * |   |   |   |
   * | 2 |   | * |
   * I3:
   * |   |   |   |
   * |   |   |   |
   * | 2 |   |   |
   * | 3 |   | * |
   * I4:
   * Now, '+' has lesser precedence than '*', implying that the operation
   * involving '*' needs to be evaluated early on, so we pop from op stack and
   * push to val stack until either the stack is empty or cur op's precedence
   * is
   * >= last op's precedence
   * |   |   |   |
   * | 2 |   |   |
   * | 3 |   |   |
   * | * |   | + |
   * Pop all from op stack and push to val stack
   * |  2  |
   * |  3  |
   * |  *  |
   * |  4  |
   * |  +  |
   */

  /**
   * We will start with creating a precedence table
   */
  std::map<const char *, int> precedence_table
      = { { "-", 10 }, { "+", 20 }, { "*", 30 },
          { "/", 30 }, { "(", 40 }, { ")", 40 } };

  Vec<AVBase *> op_stack;
  Vec<AVBase *> val_stack;

  for (AVBase *&i : vals)
    {
      switch (i->get_type ())
        {
        case AVTypeEnum::Operator:
          {
            if (!op_stack.get_size ())
              op_stack.push_back (i);
            else
              {
                while (op_stack.get_size ()
                       && precedence_table[static_cast<AVOperator *> (i)
                                               ->get_op ()]
                              <= precedence_table[static_cast<AVOperator *> (
                                                      op_stack.back ())
                                                      ->get_op ()])
                  val_stack.push_back (op_stack.pop_back ());
              }
          }
          break;

        case AVTypeEnum::Val:
          {
            val_stack.push_back (i);
          }
          break;

        default:
          break;
        }
    }

  while (op_stack.get_size ())
    val_stack.push_back (op_stack.pop_back ());

  return Arithmetic (val_stack);
}
} // namespace sf

// Modified from https://github.com/kimwalisch/calculator
///
/// @file      calculator.hpp
/// @brief     calculator::eval(const std::string&) evaluates an integer
///            arithmetic expression and returns the result. If an error
///            occurs a calculator::error exception is thrown.
///            <https://github.com/kimwalisch/calculator>
/// @author    Kim Walisch, <kim.walisch@gmail.com>
/// @copyright Copyright (C) 2013-2018 Kim Walisch
/// @license   BSD 2-Clause, https://opensource.org/licenses/BSD-2-Clause
/// @version   1.4
///
/// == Supported operators ==
///
/// OPERATOR    NAME                     ASSOCIATIVITY    PRECEDENCE
///
/// |           Bitwise Inclusive OR    Left               4
/// ^           Bitwise Exclusive OR    Left               5
/// &           Bitwise AND             Left               6
/// <<          Shift Left              Left               9
/// >>          Shift Right             Left               9
/// +           Addition                Left              10
/// -           Subtraction             Left              10
/// *           Multiplication          Left              20
/// /           Division                Left              20
/// %           Modulo                  Left              20
/// **          Raise to power          Right             30
/// e, E        Scientific notation     Right             40
/// ~           Unary complement        Left              99
///
/// The operator precedence has been set according to (uses the C and
/// C++ operator precedence): https://en.wikipedia.org/wiki/Order_of_operations
/// Operators with higher precedence are evaluated before operators
/// with relatively lower precedence. Unary operators are set to have
/// the highest precedence, this is not strictly correct for the power
/// operator e.g. "-3**2" = 9 but a lot of software tools (Bash shell,
/// Microsoft Excel, GNU bc, ...) use the same convention.
///
/// == Examples of valid expressions ==
///
/// "65536 >> 15"                       = 2
/// "2**16"                             = 65536
/// "(0 + 0xDf234 - 1000)*3/2%999"      = 828
/// "-(2**2**2**2)"                     = -65536
/// "(0 + ~(0xDF234 & 1000) *3) /-2"    = 817
/// "(2**16) + (1 << 16) >> 0X5"        = 4096
/// "5*-(2**(9+7))/3+5*(1 & 0xFf123)"   = -109221
///
/// == About the algorithm used ==
///
/// calculator::eval(std::string&) relies on the ExpressionParser
/// class which is a simple C++ operator precedence parser with infix
/// notation for integer arithmetic expressions.
/// ExpressionParser has its roots in a JavaScript parser published
/// at: http://stackoverflow.com/questions/28256/equation-expression-parser-with-precedence/114961#114961
/// The same author has also published an article about his operator
/// precedence algorithm at PerlMonks:
/// http://www.perlmonks.org/?node_id=554516
///

#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <cctype>
#include <cstddef>
#include <memory>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>

#ifdef __SWITCH__
#include <switch.h>
#endif

#ifdef YUZU
#include "yuzuSyscalls.hpp"
#endif

namespace calculator {

	/// calculator::eval() throws a calculator::error if it fails
	/// to evaluate the expression string.
	///
	class error : public std::runtime_error {
	public:
		error(const std::string& expr, const std::string& message)
			: std::runtime_error(message)
			, expr_(expr) { }
#if __cplusplus < 201103L
		~error() throw() { }
#endif
		std::string expression() const {
			return expr_;
		}

	private:
		std::string expr_;
	};

	template <typename T> class ExpressionParser {
	public:
		/// Evaluate an integer arithmetic expression and return its result.
		/// @throw error if parsing fails.
		///
		T eval(const std::string& expr) {
			T result = 0;
			index_   = 0;
			expr_    = expr;
			try {
				result = parseExpr();
				if(!isEnd())
					unexpected();
			} catch(const calculator::error&) {
				while(!stack_.empty())
					stack_.pop();
				throw;
			}
			return result;
		}

		/// Get the integer value of a character.
		T eval(char c) {
			std::string expr(1, c);
			return eval(expr);
		}

#ifdef __SWITCH__
		// Set application debug handle
		void setApplicationDebugHandle(Handle handle) {
			applicationDebug = handle;
		}
#endif

#ifdef YUZU
		void setYuzuSyscalls(std::shared_ptr<Syscalls> yuzu) {
			yuzuSyscalls = yuzu;
		}
#endif

	private:
		enum {
			OPERATOR_NULL,
			OPERATOR_BITWISE_OR,     /// |
			OPERATOR_BITWISE_XOR,    /// ^
			OPERATOR_BITWISE_AND,    /// &
			OPERATOR_BITWISE_SHL,    /// <<
			OPERATOR_BITWISE_SHR,    /// >>
			OPERATOR_ADDITION,       /// +
			OPERATOR_SUBTRACTION,    /// -
			OPERATOR_MULTIPLICATION, /// *
			OPERATOR_DIVISION,       /// /
			OPERATOR_MODULO,         /// %
			OPERATOR_POWER,          /// **
			OPERATOR_EXPONENT        /// e, E
		};

		struct Operator {
			/// Operator, one of the OPERATOR_* enum definitions
			int op;
			int precedence;
			/// 'L' = left or 'R' = right
			int associativity;
			Operator(int opr, int prec, int assoc)
				: op(opr)
				, precedence(prec)
				, associativity(assoc) { }
		};

		struct OperatorValue {
			Operator op;
			T value;
			OperatorValue(const Operator& opr, T val)
				: op(opr)
				, value(val) { }
			int getPrecedence() const {
				return op.precedence;
			}
			bool isNull() const {
				return op.op == OPERATOR_NULL;
			}
		};

		/// Expression string
		std::string expr_;
		/// Current expression index, incremented whilst parsing
		std::size_t index_;
		/// The current operator and its left value
		/// are pushed onto the stack if the operator on
		/// top of the stack has lower precedence.
		std::stack<OperatorValue> stack_;

#ifdef __SWITCH__
		// Debug handle for memory reading
		Handle applicationDebug;
#endif

#ifdef YUZU
		std::shared_ptr<Syscalls> yuzuSyscalls;
#endif

		/// Exponentiation by squaring, x^n.
		static T pow(T x, T n) {
			T res = 1;

			while(n > 0) {
				if(n % 2 != 0) {
					res *= x;
					n -= 1;
				}
				n /= 2;

				if(n > 0)
					x *= x;
			}

			return res;
		}

		T checkZero(T value) const {
			if(value == 0) {
				std::string divOperators("/%");
				std::size_t division = expr_.find_last_of(divOperators, index_ - 2);
				std::ostringstream msg;
				msg << "Parser error: division by 0";
				if(division != std::string::npos)
					msg << " (error token is \"" << expr_.substr(division, expr_.size() - division) << "\")";
				throw calculator::error(expr_, msg.str());
			}
			return value;
		}

		T calculate(T v1, T v2, const Operator& op) const {
			switch(op.op) {
			case OPERATOR_BITWISE_OR:
				return v1 | v2;
			case OPERATOR_BITWISE_XOR:
				return v1 ^ v2;
			case OPERATOR_BITWISE_AND:
				return v1 & v2;
			case OPERATOR_BITWISE_SHL:
				return v1 << v2;
			case OPERATOR_BITWISE_SHR:
				return v1 >> v2;
			case OPERATOR_ADDITION:
				return v1 + v2;
			case OPERATOR_SUBTRACTION:
				return v1 - v2;
			case OPERATOR_MULTIPLICATION:
				return v1 * v2;
			case OPERATOR_DIVISION:
				return v1 / checkZero(v2);
			case OPERATOR_MODULO:
				return v1 % checkZero(v2);
			case OPERATOR_POWER:
				return pow(v1, v2);
			case OPERATOR_EXPONENT:
				return v1 * pow(10, v2);
			default:
				return 0;
			}
		}

		bool isEnd() const {
			return index_ >= expr_.size();
		}

		/// Returns the character at the current expression index or
		/// 0 if the end of the expression is reached.
		///
		char getCharacter() const {
			if(!isEnd())
				return expr_[index_];
			return 0;
		}

		/// Parse str at the current expression index.
		/// @throw error if parsing fails.
		///
		void expect(const std::string& str) {
			if(expr_.compare(index_, str.size(), str) != 0)
				unexpected();
			index_ += str.size();
		}

		void unexpected() const {
			std::ostringstream msg;
			msg << "Syntax error: unexpected token \"" << expr_.substr(index_, expr_.size() - index_) << "\" at index " << index_;
			throw calculator::error(expr_, msg.str());
		}

		/// Eat all white space characters at the
		/// current expression index.
		///
		void eatSpaces() {
			while(std::isspace(getCharacter()) != 0)
				index_++;
		}

		/// Parse a binary operator at the current expression index.
		/// @return Operator with precedence and associativity.
		///
		Operator parseOp() {
			eatSpaces();
			switch(getCharacter()) {
			case '|':
				index_++;
				return Operator(OPERATOR_BITWISE_OR, 4, 'L');
			case '^':
				index_++;
				return Operator(OPERATOR_BITWISE_XOR, 5, 'L');
			case '&':
				index_++;
				return Operator(OPERATOR_BITWISE_AND, 6, 'L');
			case '<':
				expect("<<");
				return Operator(OPERATOR_BITWISE_SHL, 9, 'L');
			case '>':
				expect(">>");
				return Operator(OPERATOR_BITWISE_SHR, 9, 'L');
			case '+':
				index_++;
				return Operator(OPERATOR_ADDITION, 10, 'L');
			case '-':
				index_++;
				return Operator(OPERATOR_SUBTRACTION, 10, 'L');
			case '/':
				index_++;
				return Operator(OPERATOR_DIVISION, 20, 'L');
			case '%':
				index_++;
				return Operator(OPERATOR_MODULO, 20, 'L');
			case '*':
				index_++;
				if(getCharacter() != '*')
					return Operator(OPERATOR_MULTIPLICATION, 20, 'L');
				index_++;
				return Operator(OPERATOR_POWER, 30, 'R');
			case 'e':
				index_++;
				return Operator(OPERATOR_EXPONENT, 40, 'R');
			case 'E':
				index_++;
				return Operator(OPERATOR_EXPONENT, 40, 'R');
			default:
				return Operator(OPERATOR_NULL, 0, 'L');
			}
		}

		static T toInteger(char c) {
			if(c >= '0' && c <= '9')
				return c - '0';
			if(c >= 'a' && c <= 'f')
				return c - 'a' + 0xa;
			if(c >= 'A' && c <= 'F')
				return c - 'A' + 0xa;
			T noDigit = 0xf + 1;
			return noDigit;
		}

		T getInteger() const {
			return toInteger(getCharacter());
		}

		T parseDecimal() {
			T value = 0;
			for(T d; (d = getInteger()) <= 9; index_++)
				value = value * 10 + d;
			return value;
		}

		T parseHex() {
			index_  = index_ + 2;
			T value = 0;
			for(T h; (h = getInteger()) <= 0xf; index_++)
				value = value * 0x10 + h;
			return value;
		}

		bool isHex() const {
			if(index_ + 2 < expr_.size()) {
				char x = expr_[index_ + 1];
				char h = expr_[index_ + 2];
				return (std::tolower(x) == 'x' && toInteger(h) <= 0xf);
			}
			return false;
		}

		/// Parse an integer value at the current expression index.
		/// The unary `+', `-' and `~' operators and opening
		/// parentheses `(' cause recursion.
		///
		T parseValue() {
			T val = 0;
			eatSpaces();
			switch(getCharacter()) {
			case '0':
				if(isHex())
					val = parseHex();
				else
					val = parseDecimal();
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				val = parseDecimal();
				break;
			case '(':
				index_++;
				val = parseExpr();
				eatSpaces();
				if(getCharacter() != ')') {
					if(!isEnd())
						unexpected();
					throw calculator::error(expr_, "Syntax error: `)` expected at end of expression");
				}
				index_++;
				break;
			case '[':
				index_++;

				uint64_t newAddr;
#ifdef __SWITCH__
				// Assume the app is debugged
				svcReadDebugProcessMemory(&newAddr, applicationDebug, parseExpr(), sizeof(uint64_t));
#endif
#ifdef YUZU
				yuzu_rom_readbytes(yuzuInstance, &newAddr, parseExpr(), sizeof(uint64_t));
#endif

				val = newAddr;
				eatSpaces();
				if(getCharacter() != ']') {
					if(!isEnd())
						unexpected();
					throw calculator::error(expr_, "Syntax error: `]` expected at end of expression");
				}
				index_++;
				break;
			case '~':
				index_++;
				val = ~parseValue();
				break;
			case '+':
				index_++;
				val = parseValue();
				break;
			case '-':
				index_++;
				val = parseValue() * static_cast<T>(-1);
				break;
			default:
				if(!isEnd())
					unexpected();
				throw calculator::error(expr_, "Syntax error: value expected at end of expression");
			}
			return val;
		}

		/// Parse all operations of the current parenthesis
		/// level and the levels above, when done
		/// return the result (value).
		///
		T parseExpr() {
			stack_.push(OperatorValue(Operator(OPERATOR_NULL, 0, 'L'), 0));
			// first parse value on the left
			T value = parseValue();

			while(!stack_.empty()) {
				// parse an operator (+, -, *, ...)
				Operator op(parseOp());
				while(op.precedence < stack_.top().getPrecedence() || (op.precedence == stack_.top().getPrecedence() && op.associativity == 'L')) {
					// end reached
					if(stack_.top().isNull()) {
						stack_.pop();
						return value;
					}
					// do the calculation ("reduce"), producing a new value
					value = calculate(stack_.top().value, value, stack_.top().op);
					stack_.pop();
				}

				// store on stack_ and continue parsing ("shift")
				stack_.push(OperatorValue(op, value));
				// parse value on the right
				value = parseValue();
			}
			return 0;
		}
	};

#ifdef __SWITCH__
	template <typename T> inline T eval(const std::string& expression, Handle handle) {
		ExpressionParser<T> parser;
		parser.setApplicationDebugHandle(handle);
		return parser.eval(expression);
	}
#endif

#ifdef YUZU
	template <typename T> inline T eval(const std::string& expression, std::shared_ptr<Syscalls> yuzu) {
		ExpressionParser<T> parser;
		parser.setYuzuSyscalls(yuzu);
		return parser.eval(expression);
	}
#endif

#ifdef __SWITCH__
	template <typename T> inline T eval(char c, Handle Handle) {
		ExpressionParser<T> parser;
		parser.setApplicationDebugHandle(Handle);
		return parser.eval(c);
	}
#endif

#ifdef YUZU
	template <typename T> inline T eval(char c, std::shared_ptr<Syscalls> yuzu) {
		ExpressionParser<T> parser;
		parser.setYuzuSyscalls(yuzu);
		return parser.eval(c);
	}
#endif

} // namespace calculator

#endif
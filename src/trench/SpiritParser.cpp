#include "SpiritParser.h"

#include <iterator>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_stream.hpp>
#include <boost/spirit/home/phoenix/bind/bind_function.hpp>

#include "Program.h"

namespace trench {

typedef boost::spirit::istream_iterator Iterator;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

namespace {

void error_handler(Iterator &first, const Iterator &last, const Iterator &err_pos, const boost::spirit::info &what) {
	std::ostringstream message;
	message << "expecting " << what << " here: " << std::string(err_pos, last);
	std::cerr << "error_handler called!" << std::endl;
	throw std::runtime_error(message.str());
}

} // anonymous namespace

template<class Iterator>
class TrenchGrammar: public qi::grammar<Iterator, qi::unused_type, ascii::space_type> {
	public:

	TrenchGrammar(): TrenchGrammar::base_type(threads, "threads") {
		using qi::alnum;
		using qi::as_string;
		using qi::fail;
		using qi::int_;
		using qi::lexeme;
		using qi::lit;
		using qi::on_error;
		using qi::phrase_parse;
		using qi::unused_type;

		Thread *currentThread = NULL;
		auto create_thread = [&](const std::string &name) { /* currentThread = program.makeThread(name); */ };

		ident %= as_string[lexeme[+alnum]];
		label %= ident;
		instruction %= lit("mfence");
		transition %= label >> "->" >> label >> ':' >> instruction >> ';';
		thread %= "thread" >> ident[create_thread] >> '{' >> *transition >> '}';
		threads %= *int_('a');

		ident.name("ident");
		label.name("label");
		instruction.name("instruction");
		transition.name("transition");
		thread.name("thread");
		threads.name("threads");

		using namespace qi::labels;
		namespace phoenix = boost::phoenix;

		on_error<fail>(threads, phoenix::bind(error_handler, _1, _2, _3, _4));
	}

	private:

	qi::rule<Iterator, std::string(), ascii::space_type> ident;
	qi::rule<Iterator, std::string(), ascii::space_type> label;
	qi::rule<Iterator, Instruction *, ascii::space_type> instruction;
	qi::rule<Iterator, qi::unused_type, ascii::space_type> transition;
	qi::rule<Iterator, qi::unused_type, ascii::space_type> thread;
	qi::rule<Iterator, qi::unused_type, ascii::space_type> threads;
};

void SpiritParser::parse(std::istream &in, Program &program) {
	in.unsetf(std::ios::skipws);

	Iterator begin(in);
	Iterator end;

	TrenchGrammar<Iterator> grammar;
	bool success = phrase_parse(begin, end, grammar, ascii::space);

	if (begin != end) {
		throw std::runtime_error("parsing failed");
	}
}

} // namespace trench

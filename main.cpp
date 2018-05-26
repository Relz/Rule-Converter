#include <iostream>
#include "InputLibrary/Input.h"
#include "Symbol/Symbol.h"

int const REQUIRED_ARGC = 1;
std::string const NO_ARGUMENT_ERROR = "Error: file with rules is not specified";

void ParseNonterminalDeclaration(Input & input, std::string & nonterminal)
{
	if (!input.ReadUntilCharacters({ '-' }, nonterminal))
	{
		throw std::runtime_error(
			nonterminal.empty()
			? "Nonterminal declaration expected"
			: "Character '-' expected after nonterminal declaration"
		);
	}
	input.SkipArgument<char>();
}

void ParseSequenceString(Input & input, std::string & sequenceString)
{
	input.ReadUntilCharacters({ '/', '\n' }, sequenceString);
	if (input.IsEndOfLine())
	{
		throw std::runtime_error("Referencing set expected");
	}
	input.SkipArgument<char>();
}

char const EMPTY_CHARACTER = 'e';
std::string const EMPTY_CHARACTER_STRING = std::string(1, EMPTY_CHARACTER);
std::string const EMPTY_CHARACTER_STRING_BORDERED =
		std::string(1, Symbol::NONTERMINAL_LEFT_BORDER) + EMPTY_CHARACTER + Symbol::NONTERMINAL_RIGHT_BORDER;

int main(int argc, char * argv[])
{
	if (argc - 1 < REQUIRED_ARGC)
	{
		std::cerr << NO_ARGUMENT_ERROR << "\n";
		return EXIT_FAILURE;
	}

	Input input(argv[1]);
	while (input.IsEndOfStream())
	{
		std::string nonterminal;
		ParseNonterminalDeclaration(input, nonterminal);

		std::string sequenceString;
		ParseSequenceString(input, sequenceString);

		bool isEmptyCharacterSequence = sequenceString == EMPTY_CHARACTER_STRING_BORDERED;

		input.SkipLine();
	}

	return EXIT_SUCCESS;
}

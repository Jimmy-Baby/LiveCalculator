# Live AST Calculator

A simple calculator that shows the output of your input expression in realtime, using abstract syntax trees. This repository includes a DirectX 12 ImGui UI, Lexer, Parser, and Interpreter.

## Lexer, Parser, and Interpreter
The Lexer is responsible for breaking down the input code into individual tokens. 
These tokens are then passed onto the Parser, which analyzes the structure of the tokens and creates an Abstract Syntax Tree (AST). 
The Interpreter then traverses the AST and evaluates the expressions to produce the final result.

## Functionality
The arithmetic language behind the UI supports most common mathematical operators. Including addition, subtraction, multiplication, and division.
It also includes support for parentheses to control the order of operations, as well as unary operations such as the negate operator (e.g. -1, -233.0, etc.)

## Usage
Clone the repository with --recursive, run one of the two VS project scripts in the root of the repository, and then build and run the main solution.
You can then input your arithmetic expressions into the UI and see the result outputted, as well as any errors.

Feel free to contribute to this repository and add new features.

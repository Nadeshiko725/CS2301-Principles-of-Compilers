std::unique_ptr<VarDeclExprAST> parseDeclaration() {
    auto loc = lexer.getLastLocation();
    std::string id;
    if (lexer.getCurToken() != tok_var)
        return parseError<VarDeclExprAST>("var", "in variable declaration");
    lexer.getNextToken();  // eat var

    // TODO: check to see if this is a variable name(identifier)
    //       If not, report the error with 'parseError', otherwise eat the variable name
    /*
     *
     *  Write your code here.
     *
     */
    std::unique_ptr<VarType> type;  // Type is optional, it can be inferred
    if (lexer.getCurToken() != tok_identifier && lexer.getCurToken() != '<')
        return parseError<VarDeclExprAST>("identifier or type", "in variable and type declaration");
    else if (lexer.getCurToken() == tok_identifier) {
        id = lexer.getId().str();
        lexer.getNextToken();  // eat identifier

        if (lexer.getCurToken() == '<') {
            type = parseType();
            if (!type)
                return nullptr;
        }
    } else if (lexer.getCurToken() == '<') {
        type = parseType();
        if (!type)
            return nullptr;
        // lexer.getNextToken(); // eatype

        if (lexer.getCurToken() != tok_identifier)
            return parseError<VarDeclExprAST>("identifier", "in variable declaration");
        id = lexer.getId().str();
        lexer.getNextToken();  // eat identifier
    }

    // if (lexer.getCurToken() == '<') {
    //   type = parseType();
    //   if (!type)
    //     return nullptr;
    // }

    if (!type)
        type = std::make_unique<VarType>();
    lexer.consume(Token('='));
    auto expr = parseExpression();
    return std::make_unique<VarDeclExprAST>(std::move(loc), std::move(id),
                                            std::move(*type), std::move(expr));
}

std::unique_ptr<ExprAST> parseIdentifierExpr() {
    /*
     * Write your code here.
     */
    std::string identifier = lexer.getId().str();
    auto loc = lexer.getLastLocation();

    if (lexer.getNextToken() != '(')
        return std::make_unique<VariableExprAST>(std::move(loc), identifier);

    lexer.consume(Token('('));

    std::vector<std::unique_ptr<ExprAST>> args;
    if (lexer.getCurToken() != ')') {
        while (true) {
            auto arg = parseExpression();
            if (!arg)
                return nullptr;
            args.push_back(std::move(arg));

            if (lexer.getCurToken() == ')')
                break;

            if (lexer.getCurToken() != ',')
                return parseError<ExprAST>(") or ,", "to close function call or input arguments");
            lexer.getNextToken();  // eat ,
        }
    }
    lexer.consume(Token(')'));

    if (identifier == "print") {
        if (args.size() != 1)
            return parseError<ExprAST>("one argument", "for print statement");
        return std::make_unique<PrintExprAST>(std::move(loc), std::move(args[0]));
    }

    return std::make_unique<CallExprAST>(std::move(loc), identifier, std::move(args));
}

int getTokPrecedence() {
    if (!isascii(lexer.getCurToken()))
        return -1;

    // Currently we consider three binary operators: '+', '-', '*'.
    // Note that the smaller the number is, the lower precedence it will have.
    switch (static_cast<char>(lexer.getCurToken())) {
    case '-':
        return 20;
    case '+':
        return 20;
    case '*':
        return 40;
    case '@':
        return 40;
    default:
        return -1;
    }
}

std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec,
                                       std::unique_ptr<ExprAST> lhs) {
    /*
     *
     *  Write your code here.
     *
     */
    while (true) {
        int tokPrec = getTokPrecedence();
        if (tokPrec < exprPrec)
            return lhs;

        int binOp = lexer.getCurToken();
        lexer.consume(Token(binOp));
        auto loc = lexer.getLastLocation();

        auto rhs = parsePrimary();
        if (!rhs)
            return parseError<ExprAST>("primary", "in binary expression");

        int nextPrec = getTokPrecedence();
        if (nextPrec < nextPrec) {
            rhs = parseBinOpRHS(tokPrec + 1, std::move(rhs));
            if (!rhs)
                return nullptr;
        }

        lhs = std::make_unique<BinaryExprAST>(std::move(loc), binOp, std::move(lhs), std::move(rhs));
    }
}
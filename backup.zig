        //LET
        if (utils.matchU32(
            lx,
            &kw.K_EN_LET,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_LET,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_LET,
        )) {
            return .Let;
            //AND
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_AND,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_AND,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_AND,
        )) {
            return .And;
            // OR
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_OR,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_OR,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_OR,
        )) {
            return .Or;
            // END
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_END,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_END,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_END,
        )) {
            return .End;
            //IF
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_IF,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_IF,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_IF,
        )) {
            return .If;
            // THEN
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_THEN,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_THEN,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_THEN,
        )) {
            return .Then;
            // ELSE
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_ELSE,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_ELSE,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_ELSE,
        )) {
            return .Else;
            // WHILE
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_WHILE,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_WHILE,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_WHILE,
        )) {
            return .PWhile;
            // NIL
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_NIL,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_NIL,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_NIL,
        )) {
            return .Nil;
            //TRUE
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_TRUE,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_TRUE,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_TRUE,
        )) {
            return .True;
            //FALSE
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_FALSE,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_FALSE,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_FALSE,
        )) {
            return .False;
            //RETURN
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_RETURN,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_RETURN,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_RETURN,
        )) {
            return .Return;
            //FUNC
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_FUNC,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_FUNC,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_FUNC,
        )) {
            return .Func;
            //IMPORT
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_IMPORT,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_IMPORT,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_IMPORT,
        )) {
            return .Import;
            //PANIC
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_PANIC,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_PANIC,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_PANIC,
        )) {
            return .Panic;
            //DO
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_DO,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_DO,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_DO,
        )) {
            return .Do;
            //BREAK
        } else if (utils.matchU32(
            lx,
            &kw.K_EN_BREAK,
        ) or utils.matchU32(
            lx,
            &kw.K_BN_BREAK,
        ) or utils.matchU32(
            lx,
            &kw.K_PN_BREAK,
        )) {
            return .Break;
        }
        return .Identifer;

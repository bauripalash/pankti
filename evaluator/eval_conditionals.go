package evaluator

import (
	"bytes"

	"go.cs.palashbauri.in/pankti/ast"
	"go.cs.palashbauri.in/pankti/object"
)

func evalIfExpr(
	iex *ast.IfExpr,
	env *object.EnvMap,
	eh *object.ErrorHelper,
	printBuff *bytes.Buffer,
	isGui bool,
) object.Obj {

	cond := Eval(iex.Cond, env, *eh, printBuff, isGui)

	if object.IsErr(cond) {
		return cond
	}

	if isTruthy(cond) {
		return Eval(iex.TrueBlock, env, *eh, printBuff, isGui)
	} else if iex.ElseBlock != nil {
		return Eval(iex.ElseBlock, env, *eh, printBuff, isGui)
	} else {
		return NULL
	}

}

func evalWhileExpr(
	wx *ast.WhileExpr,
	env *object.EnvMap,
	eh *object.ErrorHelper,
	printBuff *bytes.Buffer,
	isGui bool,
) object.Obj {
	cond := Eval(wx.Cond, env, *eh, printBuff, isGui)
	var result object.Obj
	if object.IsErr(cond) {
		return cond
	}

	for isTruthy(cond) {
		result = Eval(wx.StmtBlock, env, *eh, printBuff, isGui)
		cond = Eval(wx.Cond, env, *eh, printBuff, isGui)
	}

	return result
}

func isTruthy(obj object.Obj) bool {
	switch obj {
	case NULL:
		return false
	case TRUE:
		return true
	case FALSE:
		return false
	default:
		return true
	}
}

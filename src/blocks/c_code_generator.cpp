#include "blocks/c_code_generator.h"
#include <iomanip>
#include <limits>
#include <math.h>
#include <sstream>

namespace block {
void c_code_generator::visit(not_expr::Ptr a) {
  std::stringstream ss;
  ss << "!(";
  a->expr1->accept(this);
  ss << last;
  ss << ")";
  last = ss.str();
}

static bool expr_needs_bracket(expr::Ptr a) {
	if (isa<binary_expr>(a))
		return true;
	else if (isa<assign_expr>(a))
		return true;
	return false;
}
void c_code_generator::emit_binary_expr(binary_expr::Ptr a,
					std::string character) {
  std::stringstream ss;
  if (expr_needs_bracket(a->expr1)) {
    ss << "(";
    a->expr1->accept(this);
    ss << last;
    ss << ")";
  } else {
    a->expr1->accept(this);
    ss << last;
  }
  ss << " " << character << " ";
  if (expr_needs_bracket(a->expr2)) {
    ss << "(";
    a->expr2->accept(this);
    ss << last;
    ss << ")";
  } else {
    a->expr2->accept(this);
    ss << last;
  }
  last = ss.str();
}
void c_code_generator::visit(and_expr::Ptr a) { emit_binary_expr(a, "&&"); }
void c_code_generator::visit(bitwise_and_expr::Ptr a) { emit_binary_expr(a, "&"); }
void c_code_generator::visit(or_expr::Ptr a) { emit_binary_expr(a, "||"); }
void c_code_generator::visit(bitwise_or_expr::Ptr a) { emit_binary_expr(a, "|"); }
void c_code_generator::visit(plus_expr::Ptr a) { emit_binary_expr(a, "+"); }
void c_code_generator::visit(minus_expr::Ptr a) { emit_binary_expr(a, "-"); }
void c_code_generator::visit(mul_expr::Ptr a) { emit_binary_expr(a, "*"); }
void c_code_generator::visit(div_expr::Ptr a) { emit_binary_expr(a, "/"); }
void c_code_generator::visit(lt_expr::Ptr a) { emit_binary_expr(a, "<"); }
void c_code_generator::visit(gt_expr::Ptr a) { emit_binary_expr(a, ">"); }
void c_code_generator::visit(lte_expr::Ptr a) { emit_binary_expr(a, "<="); }
void c_code_generator::visit(gte_expr::Ptr a) { emit_binary_expr(a, ">="); }
void c_code_generator::visit(equals_expr::Ptr a) { emit_binary_expr(a, "=="); }
void c_code_generator::visit(ne_expr::Ptr a) { emit_binary_expr(a, "!="); }
void c_code_generator::visit(mod_expr::Ptr a) { emit_binary_expr(a, "%"); }
void c_code_generator::visit(var_expr::Ptr a) { last = a->var1->var_name; }
void c_code_generator::visit(int_const::Ptr a) { last = std::to_string(a->value); }
void c_code_generator::visit(double_const::Ptr a) {
  std::stringstream ss;
  ss << std::setprecision(15);
  ss << a->value;
  if (floor(a->value) == a->value)
    ss << ".0";
  last = ss.str();
}
void c_code_generator::visit(float_const::Ptr a) {
  std::stringstream ss;
  ss << std::setprecision(15);
  ss << a->value;
  if (floor(a->value) == a->value)
    ss << ".0";
  ss << "f";
  last = ss.str();
}
void c_code_generator::visit(string_const::Ptr a) {
  std::stringstream ss;
  ss << "\"" << a->value << "\"";
  last = ss.str();
}
void c_code_generator::visit(assign_expr::Ptr a) {
  std::stringstream ss;
  if (expr_needs_bracket(a->var1)) {
    ss << "(";
    a->var1->accept(this);
    ss << last;
    ss << ")";
  } else {
    a->var1->accept(this);
    ss << last;
  }
  ss << " = ";
  a->expr1->accept(this);
  ss << last;
  last = ss.str();
}
void c_code_generator::visit(expr_stmt::Ptr a) {
  std::stringstream ss;
  a->expr1->accept(this);
  ss << last;
  ss << ";";
  if (a->annotation != "")
    ss << " //" << a->annotation;  
  last = ss.str();
}
void c_code_generator::visit(stmt_block::Ptr a) {
  std::stringstream ss;
  ss << "{" << std::endl;
  curr_indent += 1;
  for (auto stmt : a->stmts) {
    printer::indent(ss, curr_indent);
    stmt->accept(this);
    ss << last;
    ss << std::endl;
  }
  curr_indent -= 1;
  printer::indent(ss, curr_indent);

  ss << "}";
  last = ss.str();
}
void c_code_generator::visit(scalar_type::Ptr type) {
  std::stringstream ss;
  switch(type->scalar_type_id) {
    case scalar_type::BOOL_TYPE: ss << "bool"; break;
    case scalar_type::SIGNED_CHAR_TYPE: ss << "signed char"; break;
    case scalar_type::SHORT_INT_TYPE: ss << "short int"; break;
    case scalar_type::UNSIGNED_SHORT_INT_TYPE: ss << "unsigned short int"; break;
    case scalar_type::INT_TYPE: ss << "int"; break;
    case scalar_type::UNSIGNED_INT_TYPE: ss << "unsigned int"; break;
    case scalar_type::LONG_INT_TYPE: ss << "long int"; break;
    case scalar_type::UNSIGNED_LONG_INT_TYPE: ss << "unsigned long int"; break;
    case scalar_type::LONG_LONG_INT_TYPE: ss << "long long int"; break;
    case scalar_type::UNSIGNED_LONG_LONG_INT_TYPE: ss << "unsigned long long int"; break;
    case scalar_type::CHAR_TYPE: ss << "char"; break;
    case scalar_type::UNSIGNED_CHAR_TYPE: ss << "unsigned char"; break;
    case scalar_type::VOID_TYPE: ss << "void"; break;
    case scalar_type::FLOAT_TYPE: ss << "float"; break;
    case scalar_type::DOUBLE_TYPE: ss << "double"; break;
    default:
      assert(false && "Invalid scalar type");
  }
  last = ss.str();
}
void c_code_generator::visit(named_type::Ptr type) {
  std::stringstream ss;
  ss << type->type_name;
  if (type->template_args.size()) {
    ss << "<";
    bool needs_comma = false;
    for (auto a: type->template_args) {
      if (needs_comma)
	ss << ", ";
      needs_comma = true;
      a->accept(this);	
      ss << last;
    }
    ss << ">";
  }
  last = ss.str();
}
void c_code_generator::visit(pointer_type::Ptr type) {
  std::stringstream ss;
  if (!isa<scalar_type>(type->pointee_type) &&
      !isa<pointer_type>(type->pointee_type) &&
      !isa<named_type>(type->pointee_type))
    assert(
	   false &&
	   "Printing pointers of complex type is not supported yet");
  type->pointee_type->accept(this);
  ss << last;
  ss << "*";
  last = ss.str();
}
void c_code_generator::visit(array_type::Ptr type) {
  std::stringstream ss;
  if (!isa<scalar_type>(type->element_type) &&
      !isa<pointer_type>(type->element_type) &&
      !isa<named_type>(type->element_type))
    assert(false &&
	   "Printing arrays of complex type is not supported yet");
  type->element_type->accept(this);
  ss << last;
  if (type->size != -1)
    ss << "[" << type->size << "]";
  else
    ss << "[]";
  last = ss.str();
}
void c_code_generator::visit(builder_var_type::Ptr type) {
  std::stringstream ss;
  if (type->builder_var_type_id == builder_var_type::DYN_VAR)
    ss << "builder::dyn_var<";
  else if (type->builder_var_type_id == builder_var_type::STATIC_VAR)
    ss << "builder::static_var<";
  type->closure_type->accept(this);
  ss << last;
  ss << ">";
  last = ss.str();
}
void c_code_generator::visit(var::Ptr var) { 
  last = var->var_name;
}

static void print_array_decl(array_type::Ptr atype, var::Ptr decl_var, c_code_generator* self, 
				    std::stringstream &append) {
  std::stringstream ss;
  append << "[";
  if (atype->size != -1)
    append << atype->size;
  append << "]";

  if (isa<array_type>(atype->element_type)) {
    print_array_decl(to<array_type>(atype->element_type), decl_var, self, append);
    ss << self->last;
  }
  else if (isa<scalar_type>(atype->element_type) || isa<pointer_type>(atype->element_type) 
	   || isa<named_type>(atype->element_type)) {
    atype->element_type->accept(self);	
    ss << self->last;
    if (decl_var->hasMetadata<std::vector<std::string>>("attributes")) {
      const auto &attributes = decl_var->getMetadata<std::vector<std::string>>("attributes");
      for (auto attr: attributes) {
	ss << " " << attr;
      }
    }
    ss << " ";
    ss << decl_var->var_name;
    ss << append.str();	
  } else {
    assert(false && "Printing arrays of complex type is not supported yet");
  }
  //  return ss.str();
  self->last = ss.str();
}

void c_code_generator::visit(decl_stmt::Ptr a) {
  std::stringstream ss;
  if (isa<function_type>(a->decl_var->var_type)) {
    function_type::Ptr type =
      to<function_type>(a->decl_var->var_type);
    type->return_type->accept(this);
    ss << last;
    ss << " (*";
    ss << a->decl_var->var_name;
    ss << ")(";
    for (unsigned int i = 0; i < type->arg_types.size(); i++) {
      type->arg_types[i]->accept(this);
      ss << last;
      if (i != type->arg_types.size() - 1)
	ss << ", ";
    }
    ss << ")";
    if (a->init_expr != nullptr) {
      ss << " = ";
      a->init_expr->accept(this);
      ss << last;
    }
    ss << ";";
    last = ss.str();
    return;
  } else if (isa<array_type>(a->decl_var->var_type)) {
    array_type::Ptr type = to<array_type>(a->decl_var->var_type);
    std::stringstream s;
    print_array_decl(type, a->decl_var, this, s);
    ss << last;
    if (a->init_expr != nullptr) {
      ss << " = ";
      a->init_expr->accept(this);
      ss << last;
    }
    ss << ";";
    last = ss.str();
    return;
  }

  a->decl_var->var_type->accept(this);
  ss << last;
  if (a->decl_var->hasMetadata<std::vector<std::string>>("attributes")) {
    const auto &attributes = a->decl_var->getMetadata<std::vector<std::string>>("attributes");
    for (auto attr: attributes) {
      ss << " " << attr;
    }
  }
  ss << " ";
  ss << a->decl_var->var_name;
  if (a->init_expr == nullptr) {
    ss << ";";
  } else {
    ss << " = ";
    a->init_expr->accept(this);
    ss << last;
    ss << ";";
  }
  last = ss.str();
}
void c_code_generator::visit(if_stmt::Ptr a) {
  std::stringstream ss;
  ss << "if (";
  a->cond->accept(this);
  ss << last;
  ss << ")";
  if (isa<stmt_block>(a->then_stmt)) {
    ss << " ";
    a->then_stmt->accept(this);
    ss << last;
    ss << " ";
  } else {
    ss << std::endl;
    curr_indent++;
    printer::indent(ss, curr_indent);
    a->then_stmt->accept(this);
    ss << last;
    ss << std::endl;
    curr_indent--;
  }

  if (isa<stmt_block>(a->else_stmt)) {
    if (to<stmt_block>(a->else_stmt)->stmts.size() == 0) {
      last = ss.str();
      return;
    }
    ss << "else";
    ss << " ";
    a->else_stmt->accept(this);
    ss << last;
  } else {
    ss << "else";
    ss << std::endl;
    curr_indent++;
    printer::indent(ss, curr_indent);
    a->else_stmt->accept(this);
    ss << last;
    curr_indent--;
  }
  last = ss.str();
}
void c_code_generator::visit(while_stmt::Ptr a) {
  std::stringstream ss;
  ss << "while (";
  a->cond->accept(this);
  ss << last;
  ss << ")";
  if (isa<stmt_block>(a->body)) {
    ss << " ";
    a->body->accept(this);
    ss << last;
  } else {
    ss << std::endl;
    curr_indent++;
    printer::indent(ss, curr_indent);
    a->body->accept(this);
    ss << last;
    curr_indent--;
  }
  last = ss.str();
}
void c_code_generator::visit(for_stmt::Ptr a) {
  std::stringstream ss;
  ss << "for (";
  a->decl_stmt->accept(this);
  ss << last;
  ss << " ";
  a->cond->accept(this);
  ss << last;
  ss << "; ";
  a->update->accept(this);
  ss << last;
  ss << ")";
  if (isa<stmt_block>(a->body)) {
    ss << " ";
    a->body->accept(this);
    ss << last;
  } else {
    ss << std::endl;
    curr_indent++;
    printer::indent(ss, curr_indent);
    a->body->accept(this);
    ss << last;
    curr_indent--;
  }
  last = ss.str();
}
void c_code_generator::visit(break_stmt::Ptr a) { last = "break;"; }
void c_code_generator::visit(continue_stmt::Ptr a) { last = "continue;"; }
void c_code_generator::visit(sq_bkt_expr::Ptr a) {
  std::stringstream ss;
  if (expr_needs_bracket(a->var_expr)) {
    ss << "(";
  }
  a->var_expr->accept(this);
  ss << last;
  if (expr_needs_bracket(a->var_expr)) {
    ss << ")";
  }
  ss << "[";
  a->index->accept(this);
  ss << last;
  ss << "]";
  last = ss.str();
}
void c_code_generator::visit(function_call_expr::Ptr a) {
  std::stringstream ss;
  if (expr_needs_bracket(a->expr1)) {
    ss << "(";
  }
  a->expr1->accept(this);
  ss << last;
  if (expr_needs_bracket(a->expr1)) {
    ss << ")";
  }
  ss << "(";
  for (unsigned int i = 0; i < a->args.size(); i++) {
    a->args[i]->accept(this);
    ss << last;
    if (i != a->args.size() - 1)
      ss << ", ";
  }
  ss << ")";
  last = ss.str();
}
void c_code_generator::visit(initializer_list_expr::Ptr a) {
  std::stringstream ss;
  ss << "{";
  for (unsigned int i = 0; i < a->elems.size(); i++) {
    a->elems[i]->accept(this);
    ss << last;
    if (i != a->elems.size() - 1)
      ss << ", ";
  }
  ss << "}";
  last = ss.str();
}
void c_code_generator::handle_func_arg(var::Ptr a) {
  std::stringstream ss;
  function_type::Ptr type =
    to<function_type>(a->var_type);
  type->return_type->accept(this);
  ss << last;
  ss << " (*";
  ss << a->var_name;
  ss << ")(";
  for (unsigned int i = 0; i < type->arg_types.size(); i++) {
    type->arg_types[i]->accept(this);
    ss << last;
    if (i != type->arg_types.size() - 1)
      ss << ", ";
  }
  ss << ")";
  last = ss.str();
}
void c_code_generator::visit(func_decl::Ptr a) {
  std::stringstream ss;
  a->return_type->accept(this);
  ss << last;
  if (a->hasMetadata<std::vector<std::string>>("attributes")) {
    const auto &attributes = a->getMetadata<std::vector<std::string>>("attributes");
    for (auto attr: attributes) {
      ss << " " << attr;
    }
  }
  ss << " " << a->func_name;
  ss << " (";
  bool printDelim = false;
  for (auto arg : a->args) {
    if (printDelim)
      ss << ", ";
    printDelim = true;
    if (isa<function_type>(arg->var_type)) {
      handle_func_arg(arg);
    } else {
      arg->var_type->accept(this);
      ss << last;
      ss << " " << arg->var_name;
    }
  }
  if (!printDelim)
    ss << "void";
  ss << ")";
  if (isa<stmt_block>(a->body)) {
    ss << " ";
    a->body->accept(this);
    ss << last;
    ss << std::endl;
  } else {
    ss << std::endl;
    curr_indent++;
    printer::indent(ss, curr_indent);
    a->body->accept(this);
    ss << last;
    ss << std::endl;
    curr_indent--;
  }
  last = ss.str();
}
void c_code_generator::visit(goto_stmt::Ptr a) { 
  std::stringstream ss;
  //a->dump(ss, 1); 
  ss << "goto ";
  ss << a->label1->label_name << ";";
  last = ss.str();
}
void c_code_generator::visit(label_stmt::Ptr a) { 
  std::stringstream ss;
  ss << a->label1->label_name << ":";
  last = ss.str();
}
void c_code_generator::visit(return_stmt::Ptr a) {
  std::stringstream ss;
  ss << "return ";
  a->return_val->accept(this);
  ss << last;
  ss << ";";
  last = ss.str();
}
void c_code_generator::visit(member_access_expr::Ptr a) {
  std::stringstream ss;
  if (isa<sq_bkt_expr>(a->parent_expr)) {
    sq_bkt_expr::Ptr parent = to<sq_bkt_expr>(a->parent_expr);
    if (isa<int_const>(parent->index)) {
      auto index = to<int_const>(parent->index);
      if (index->value == 0) {
	if (!isa<var_expr>(parent->var_expr)) {
	  ss << "(";
	}
	parent->var_expr->accept(this);
	ss << last;
	if (!isa<var_expr>(parent->var_expr)) {
	  ss << ")";
	}
	ss << "->" << a->member_name;	
	last = ss.str();
	return;
      }
    }
  }

  if (!isa<var_expr>(a->parent_expr))
    ss << "(";
  a->parent_expr->accept(this);
  ss << last;
  if (!isa<var_expr>(a->parent_expr))
    ss << ")";

  ss << "." << a->member_name;
  last = ss.str();
}
void c_code_generator::visit(addr_of_expr::Ptr a) {
  std::stringstream ss;
  ss << "(&(";
  a->expr1->accept(this);
  ss << last;
  ss << "))";
  last = ss.str();
}
} // namespace block

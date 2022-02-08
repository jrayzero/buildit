#ifndef BUILDER_DYN_VAR_H
#define BUILDER_DYN_VAR_H

#include "builder/builder.h"
namespace builder {

class var {
public:
	// Optional var name
	std::string var_name;
	block::var::Ptr block_var;
	block::decl_stmt::Ptr block_decl_stmt;

        // Feature to implement members
        bool is_member = false;
        var *parent_var;

	static block::type::Ptr create_block_type(void) {
		// Cannot create block type for abstract class
		assert(false);
	}

	var() = default;

	explicit operator bool();

	// This is for enabling dynamic inheritance
	virtual ~var() = default;


};

// Struct to initialize a dyn_var as member;
struct as_member_of {
	var *parent_var;
	std::string member_name; 
	as_member_of(var* p, std::string n): parent_var(p), member_name(n) {};	
};

template<typename T>
class dyn_var: public var{
public:
	
	typedef builder BT;
	typedef dyn_var<T> my_type;
	typedef BT associated_BT;
	typedef T stored_type;
	typedef my_type super;
		
	template <typename... types>
	BT operator()(const types &... args) {
		return ((BT) * this)(args...);
	}

	
	// These three need to be defined inside the class, cannot be defined globally
	BT operator=(const var &a) { 
		return (BT) * this = a; 
	}

	BT operator[] (const BT &a) {
		return ((BT) *this)[a];
	}
	BT operator=(const BT &a) {
		return (BT)*this = a;
	}

	BT operator=(const dyn_var<T> &a) {
		return (BT) * this = a;
	}

	template <typename TO>
	BT operator=(const dyn_var<TO> &a) {
		return (BT) * this = a;
	}

	BT operator=(const int &a) { 
		return operator=((BT)a); 
	}

	BT operator=(const double &a) { 
		return operator=((BT)a); 
	}

	template <typename Ts>
	BT operator=(const static_var<Ts> &a) {
		return operator=((BT)a);
	}

	BT operator!() { return !(BT) * this; }
	operator bool() { return (bool)(BT) * this; }

	static block::type::Ptr create_block_type(void) { 
		return type_extractor<T>::extract_type(); 
	}

	void create_dyn_var(bool create_without_context = false) {
		if (create_without_context) {
			block::var::Ptr dyn_var = std::make_shared<block::var>();
			dyn_var->var_type = create_block_type();
			block_var = dyn_var;
			return;
		}
		assert(builder_context::current_builder_context != nullptr);
		assert(builder_context::current_builder_context->current_block_stmt != nullptr);
		builder_context::current_builder_context->commit_uncommitted();
		block::var::Ptr dyn_var = std::make_shared<block::var>();
		dyn_var->var_type = create_block_type();
		block_var = dyn_var;
		tracer::tag offset = get_offset_in_function();
		dyn_var->static_offset = offset;
		block_decl_stmt = nullptr;
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;
		block::decl_stmt::Ptr decl_stmt = std::make_shared<block::decl_stmt>();
		decl_stmt->static_offset = offset;
		decl_stmt->decl_var = dyn_var;
		decl_stmt->init_expr = nullptr;
		block_decl_stmt = decl_stmt;
		builder_context::current_builder_context->add_stmt_to_current_block(decl_stmt);
	}
	// Basic and other constructors
	dyn_var(const char* name=nullptr) { 
		if (builder_context::current_builder_context == nullptr) {
			create_dyn_var(true); 
			if (name != nullptr) {
				block_var->var_name = name;
				var_name = name;
			}
		} else
			create_dyn_var(false); 
	}
	dyn_var(const dyn_var_sentinel_type& a, std::string name = "") {
		create_dyn_var(true);
		if (name != "") {
			block_var->var_name = name;
			var_name = name;
		}
	
	}
        // Constructor to initialize a dyn_var as member
        // This declaration does not produce a declaration
        dyn_var(const as_member_of &a) {
		is_member = true;
		parent_var = a.parent_var;
		var_name = a.member_name;            
		block_var = nullptr;
		block_decl_stmt = nullptr;
	}
	// A very special move constructor that is used to create exact 
	// replicas of variables
	dyn_var(const dyn_var_consume &a) {
		block_var = a.block_var;
		var_name = block_var->var_name;
		block_decl_stmt = nullptr;	
	}

	dyn_var(const my_type &a) : my_type((BT)a) {}

	
	template <typename TO>
	dyn_var(const dyn_var<TO> &a) : my_type((BT)a) {}
	
	template <typename TO>
	dyn_var(const static_var<TO> &a) : my_type((TO)a) {}

	dyn_var(const BT &a) {
		builder_context::current_builder_context->remove_node_from_sequence(a.block_expr);
		create_dyn_var();
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;
		block_decl_stmt->init_expr = a.block_expr;
	}

	dyn_var(const int &a) : my_type((BT)a) {}
	dyn_var(const bool &a) : my_type((BT)a) {}
	dyn_var(const double &a) : my_type((BT)a) {}
	dyn_var(const float &a) : my_type((BT)a) {}

	dyn_var(const std::initializer_list<BT> &_a) {
		std::vector<BT> a(_a);

		assert(builder_context::current_builder_context != nullptr);
		for (unsigned int i = 0; i < a.size(); i++) {
			builder_context::current_builder_context->remove_node_from_sequence(a[i].block_expr);
		}
		create_dyn_var();
		if (builder_context::current_builder_context->bool_vector.size() > 0)
			return;

		tracer::tag offset = get_offset_in_function();
		block::initializer_list_expr::Ptr list_expr = std::make_shared<block::initializer_list_expr>();
		list_expr->static_offset = offset;
		for (unsigned int i = 0; i < a.size(); i++) {
			list_expr->elems.push_back(a[i].block_expr);
		}
		block_decl_stmt->init_expr = list_expr;
	}

	virtual ~dyn_var() = default;

	dyn_var* addr(void) {
		return this;
	}

};



template <typename T>
typename std::enable_if<std::is_base_of<var, T>::value>::type create_return_stmt(const T &a) {
	create_return_stmt((typename T::associated_BT)a);	
}


}

#endif

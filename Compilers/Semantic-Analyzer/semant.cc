#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"
#include <vector>

extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}

/* 
 * Symbol tables for semantic checking. 
 * function table stores the method name as symbol, and the related feature for the method as data.
 * attribute table stores the attribute name as symbol, and the type as data.
 */
SymbolTable<Symbol, Feature> *function_table;
SymbolTable<Symbol, Symbol> *attribute_table;


/* pointer to the classtable, declared globally to access the semant_error() in the entire scope. */
ClassTable *classtable;


/*
 * This function is the constructor for classtable declared in semant.h
 * It creates an inheritance graph for the classes.
 * It then checks for any cycles and undefinded classes and reports errors
 */
ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) {

    /* adding basic classes to the inheritance graph. */
    install_basic_classes();
    
    /* flag to check if the Main class is present. */
    int is_Main_present = 0;

    /* flag to check if any error was encountered. */
    int is_error = 0;

    /* iterator for the map that stores the inheritance graph. */
    std::map<Symbol, Class_>::iterator it;

    /* iterating over the class list and populating the inheritance graph. */
    for(int i=classes->first(); classes->more(i); i=classes->next(i))
    {
        /* obtaining the current class. */
        Class_ current_class = classes->nth(i);

        /* obtaining the current class name and its parent. */
        Symbol current_class_name = current_class->get_name();
        Symbol current_class_parent = current_class->get_parent();

        /* checking if the class is already present. */
        it = inheritance_graph.find(current_class_name);
        if(it!=inheritance_graph.end())
        {
            semant_error(current_class)<<"Class "<<current_class_name<<" was previously defined.\n";
        }

        /* checking if the class doesnt inherit itself. */
        if(current_class_name==current_class_parent)
        {
            semant_error(current_class)<<"Class "<<current_class_name<<", or an ancestor of "<<current_class_name<<", is involved in an inheritance cycle"<<endl;
        }

        /* checking if the class name is not SELF_TYPE */
        else if(current_class_name==SELF_TYPE)
        {
            semant_error(current_class)<<"Redifination of basic class SELF_TYPE\n";   
        }

        /* checking if the class doesn't inherit the basic types. */
        else if(current_class_parent==Bool || current_class_parent==Int || current_class_parent==Str || current_class_parent==SELF_TYPE)
        {
            semant_error(current_class)<<"Class "<<current_class_name<<" cannot inherit class "<<current_class_parent<<".\n";
        }

        /* inserting the current class name, and the associated class object in the map. */
        else
            inheritance_graph.insert(std::pair<Symbol, Class_>(current_class_name, current_class));
        
        /* checking if the current class is Main and setting the appropriate flag. */
        if(current_class_name==Main)
            is_Main_present = 1;
    }

    /* if Main class not found, reporting error. */
    if(is_Main_present==0)
    {
        semant_error()<<"Class Main is not defined.\n";
    }

    /*
     * Checking for cycle in the inheritance graph.
     * We iterate over the list of classes in the graph, and check for cycle beginning from each class.
     * We use the general two pointer method for finding a cycle in a linked list.
     * Two pointers (iterators in case of map) are pointed to the current class in each iteration.
     * One pointer is made to point to its parent, and the other to the parent of the parent in each step.
     * This is done until the Object class is reached or the two iterators point to the same class.
     * A cycle is present in the latter case, and the error is reported.
     */
    for(it = inheritance_graph.begin(); it!=inheritance_graph.end(); it++)
    {
        /* checking for cycle beginning from the current class. */

        /* flag variable for finding the cycle. */  
        bool is_cycle = false;
        
        /* iterators */
        Symbol slow_iterator, fast_iterator;
        slow_iterator = fast_iterator = it->first;

        /* base case when the class is Object, no cycle can be present. */
        if(slow_iterator==Object)
            continue;

        while(1)
        {   
            /* if the parent of the first iterator is not defined, report error. */
            if(inheritance_graph.find(inheritance_graph.find(slow_iterator)->second->get_parent())==inheritance_graph.end())
            {
                semant_error(inheritance_graph.find(slow_iterator)->second)<<"Class "<<slow_iterator<<" inherits from an undefined class "<<inheritance_graph.find(slow_iterator)->second->get_parent()<<".\n";
                return;
            }
            /* advancing the first iterator. */
            slow_iterator = inheritance_graph.find(slow_iterator)->second->get_parent();
            
            /* if the parent of the second iterator is not defined, report error. */
            if(inheritance_graph.find(inheritance_graph.find(fast_iterator)->second->get_parent())==inheritance_graph.end())
            {
                semant_error(inheritance_graph.find(fast_iterator)->second)<<"Class "<<fast_iterator<<" inherits from an undefined class "<<inheritance_graph.find(fast_iterator)->second->get_parent()<<".\n";
                return;
            }
            /* advancing the second iterator. */
            fast_iterator = inheritance_graph.find(fast_iterator)->second->get_parent();
            if(fast_iterator!=Object)
            {
                if(inheritance_graph.find(inheritance_graph.find(fast_iterator)->second->get_parent())==inheritance_graph.end())
                {
                    semant_error(inheritance_graph.find(fast_iterator)->second)<<"Class "<<fast_iterator<<" inherits from an undefined class "<<inheritance_graph.find(fast_iterator)->second->get_parent()<<".\n";
                    return;
                }
                fast_iterator = inheritance_graph.find(fast_iterator)->second->get_parent();
            }

            /* if any of the iterators point to Object, no cycle is found. */
            if(slow_iterator==Object || fast_iterator==Object)
                break;

            /* if the iterators point to the same class, cycle found. */
            if(slow_iterator==fast_iterator)
            {
                /* setting the appropriate flag. */
                is_cycle = true;
                break;
            }
        }

        /* if cycle present, report error. */
        if(is_cycle)
        {
            semant_error(it->second)<<"Class "<<it->first<<", or an ancestor of "<<it->first<<", is involved in an inheritance cycle"<<endl;
        }
    }

}

/*
 * This function installs the basic classes of the COOL language.
 * It then populates the inheritance graph with the basic classes. 
 */
void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
    class_(Object, 
           No_class,
           append_Features(
                   append_Features(
                           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
                           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
                   single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
           filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
    class_(IO, 
           Object,
           append_Features(
                   append_Features(
                           append_Features(
                                   single_Features(method(out_string, single_Formals(formal(arg, Str)),
                                              SELF_TYPE, no_expr())),
                                   single_Features(method(out_int, single_Formals(formal(arg, Int)),
                                              SELF_TYPE, no_expr()))),
                           single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
                   single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
           filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
    class_(Int, 
           Object,
           single_Features(attr(val, prim_slot, no_expr())),
           filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
    class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
    class_(Str, 
           Object,
           append_Features(
                   append_Features(
                           append_Features(
                                   append_Features(
                                           single_Features(attr(val, Int, no_expr())),
                                           single_Features(attr(str_field, prim_slot, no_expr()))),
                                   single_Features(method(length, nil_Formals(), Int, no_expr()))),
                           single_Features(method(concat, 
                                      single_Formals(formal(arg, Str)),
                                      Str, 
                                      no_expr()))),
                   single_Features(method(substr, 
                              append_Formals(single_Formals(formal(arg, Int)), 
                                     single_Formals(formal(arg2, Int))),
                              Str, 
                              no_expr()))),
           filename);
    
    /* populating the inheritance graph with the basic classes. */
    inheritance_graph.insert(std::pair<Symbol, Class_>(Object, Object_class));
    inheritance_graph.insert(std::pair<Symbol, Class_>(IO, IO_class));
    inheritance_graph.insert(std::pair<Symbol, Class_>(Int, Int_class));
    inheritance_graph.insert(std::pair<Symbol, Class_>(Bool, Bool_class));
    inheritance_graph.insert(std::pair<Symbol, Class_>(Str, Str_class));
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
}

/*
 * This function returns the type of the least common ancestor of two classes.
 * Least common ancestor represents the first common inherited class in the inherited graph.
 * It is also called the join of two classes (as mentioned in the COOL Manual).
 * @param then_type: the type of the first class.
 * @param else_type: the type of the second class. 
 */
Symbol get_least_common_ancestor_type(Symbol then_type, Symbol else_type)
{
    /* obtaining the inheritance graph. */
    std::map<Symbol, Class_>inheritance_graph = classtable->get_inheritance_graph();
    
    /* iterating over the inheritance list for first class, and iterating the entire inheritance list of the second class. */
    Symbol then_type_temp = then_type, else_type_temp = else_type;

    /* while the two type dont match. */
    while(then_type_temp!=else_type_temp)
    {   
        /* traversing the inheritance list of second class. */
        while(else_type_temp!=Object)
        {
            else_type_temp = inheritance_graph.find(else_type_temp)->second->get_parent();

            /* if the types match, break out of the loop. */
            if(then_type_temp==else_type_temp)
                break;
        }

        /* if types match, break out of the loop. */
        if(then_type_temp==else_type_temp)
            break;

        then_type_temp = inheritance_graph.find(then_type_temp)->second->get_parent();
        else_type_temp = else_type;
    }

    return then_type_temp;
}


/*
 * This function checks if the first class inherits the (directly or indirectly) second class.
 */
bool check_ancestor(Symbol child, Symbol parent)
{
    /* obtaining the inheritance graph. */
    std::map<Symbol, Class_> inheritance_graph = classtable->get_inheritance_graph();

    /* iterating while the second class type is reached in the inheritance list. */
    while(child!=parent)
    {
        /* if child becomes object, it cant inherit the second class */
        if(child==Object)
            break;

        child = inheritance_graph.find(child)->second->get_parent();
    }

    /* if types match, first class is the subclass of the second, return true. */
    if(child==parent)
        return true;

    return false;
}


/*
 * This function returns the feature associated with a method name.
 * The feature can be present in the immediate class, or any of the inherited classes.
 * If the feature is not found, the function returns NULL.
 * @param method_name: the method name to be looked for.
 * @param child_class: the current class to look for the method.
 */
Feature get_method_feature(Symbol method_name, Symbol child_class)
{
    /* No_class can't have the method. */
    if(child_class==No_class)
    {
        return NULL;
    }

    /* obtaining the inheritance graph. */
    std::map<Symbol, Class_>inheritance_graph = classtable->get_inheritance_graph();

    /* obtaining the class object and the feature list of the current class. */
    Class_ current_class = inheritance_graph.find(child_class)->second;
    Features features = current_class->get_features();

    /* iterating over the features, to look for a match for the method name. */
    for(int i=features->first(); features->more(i); i=features->next(i))
    {
        Feature current_feature = features->nth(i);

        /* if a match is found, return the corresponding feature. */
        if(current_feature->is_method() && current_feature->get_name()==method_name)
            return current_feature;
    }

    /* if feature is not found, look for the method in the parent class. */
    return get_method_feature(method_name, inheritance_graph.find(child_class)->second->get_parent());
}


/*
 * This function returns the type of the assign expression.
 * As mentioned in the COOL manual,
 * It first evaluates the type of the left and right expressions.
 * It then reports error, if the types do the both sides of the assign don't match.
 * The return type is the type of the assigned type.
 */ 
Symbol assign_class::get_expression_type(Class_ cur_class)
{
    /* evaluating the type of the left expression. */
    Symbol *left_type = attribute_table->lookup(name);
    if(left_type==NULL)
    {
        classtable->semant_error(cur_class)<<"Assignment to undeclared variable "<<name<<".\n";
        type = Object;
        return Object;
    }

    /* evaluating the type of the right expresion. */
    Symbol right_type = expr->get_expression_type(cur_class);

    /* checking if the types match. */
    if(*left_type!=right_type)
    {
        classtable->semant_error(cur_class)<<"Type "<<right_type<<" of assigned expression does not conform to declared type "<<*left_type<<" of identifier "<<name<<".\n";
        type = Object;
        return Object;
    }

    /* returning the type of the assign expression. */
    type = *left_type;
    return *left_type;
}


/*
 * This fucntion returns the type of the static dispatch expression. 
 * According to COOL manual,
 * The type of the expression before the '@' symbol is evaluated first.
 * The type of the expression should be a sub class of the type of the dispatch, which is checked.
 * Then the feature associated with the method is obtained, if the method is undefined, error is reported.
 * The actual parameters of the call are then matched to the formal parameters.
 * The return type is then evaluated. 
 */
Symbol static_dispatch_class::get_expression_type(Class_ cur_class)
{
    /* evaulating the first expression. */
    Symbol child_type = expr->get_expression_type(cur_class);
    if(child_type==SELF_TYPE)
        child_type = cur_class->get_name();

    Symbol calling_type = type_name;

    /* checking if the expression type is the subclass of calling type. */
    if(!check_ancestor(child_type, calling_type))
    {
        classtable->semant_error(cur_class)<<"Expression type "<<child_type<<" does not conform to declared static dispatch type "<<calling_type<<".\n";
        type = Object;
        return Object;
    }

    /* obtaining the feature associated with the method name. */
    Feature called_feature = get_method_feature(name, calling_type);

    /* if method is undefined the current scope, error is reported. */
    if(called_feature==NULL)    
    {
        classtable->semant_error(cur_class)<<"Dispatch to undefined method "<<name<<".\n";
        type = Object;
        return Object;
    }

    /* obtaining the formal list from the feature. */
    Formals called_formals = called_feature->get_formals();

    /* if the number of parameters dont match, reporting error. */
    if(called_formals->len()!=actual->len())
    {
        classtable->semant_error(cur_class)<<"Method "<<name<<" called with wrong number of arguments.\n";
        type = Object;
        return Object;
    }

    /* matching the list of actual and formal parameters. */
    for(int i=actual->first(); actual->more(i); i=actual->next(i))
    {
        Expression current_expression = actual->nth(i);
        Formal current_formal = called_formals->nth(i);

        /* obtainng the type of the current actual parameter. */
        Symbol current_type = current_expression->get_expression_type(cur_class);

        /* if the type is SELF_TYPE, setting it to class name. */
        if(current_type==SELF_TYPE)
            current_type = cur_class->get_name();

        /* if the current actual parameter does not conform to the formal parameter, report error. */
        if(!(current_type==current_formal->get_type() || check_ancestor(current_type, current_formal->get_type())))
        {
            classtable->semant_error(cur_class)<<"In call of method "<<name<<", type "<<current_expression->get_expression_type(cur_class)<<" of parameter "<<current_formal->get_name()<<" does not conform to declared type "<<current_formal->get_type()<<".\n";
            type = Object;
            return Object;
        }
    }

    /* obtaing the return type of the dispatch. */
    Symbol return_type = called_feature->get_return_type();

    /* if the type is SELF_TYPE, setting it to class name. */
    if(return_type==SELF_TYPE)
        return_type = expr->get_expression_type(cur_class);

    type = return_type;
    return return_type;
}


/*
 * This fucntion returns the type of the dispatch expression. 
 * According to COOL manual,
 * The type of the expression calling the method is evaluated first.
 * Then the feature associated with the method is obtained, if the method is undefined, error is reported.
 * The actual parameters of the call are then matched to the formal parameters.
 * The return type is then evaluated. 
 */
Symbol dispatch_class::get_expression_type(Class_ cur_class)
{
    /* evaluating the type of the expression. */
    Symbol calling_type = expr->get_expression_type(cur_class);

    /* if the type is SELF_TYPE, setting it to the current class name. */
    if(calling_type==SELF_TYPE)
        calling_type = cur_class->get_name();

    /* obtaining the feature of the method. */
    Feature called_feature = get_method_feature(name, calling_type);

    /* if the number of parameters dont match, reporting error. */
    if(called_feature==NULL)
    {
        classtable->semant_error(cur_class)<<"Dispatch to undefined method "<<name<<".\n";
        type = Object;
        return Object;
    }

    /* obtaining the formal list from the feature. */
    Formals called_formals = called_feature->get_formals();

    /* if the number of parameters dont match, reporting error. */
    if(called_formals->len()!=actual->len())
    {
        classtable->semant_error(cur_class)<<"Method "<<name<<" called with wrong number of arguments.\n";
        type = Object;
        return Object;
    }

    /* matching the list of actual and formal parameters. */
    for(int i=actual->first(); actual->more(i); i=actual->next(i))
    {
        Expression current_expression = actual->nth(i);
        Formal current_formal = called_formals->nth(i);

        /* obtainng the type of the current actual parameter. */
        Symbol current_type = current_expression->get_expression_type(cur_class);

        /* if the type is SELF_TYPE, setting it to class name. */
        if(current_type==SELF_TYPE)
            current_type = cur_class->get_name();

        /* if the current actual parameter does not conform to the formal parameter, report error. */
        if(!(current_type==current_formal->get_type() || check_ancestor(current_type, current_formal->get_type())))
        {
            classtable->semant_error(cur_class)<<"In call of method "<<name<<", type "<<current_expression->get_expression_type(cur_class)<<" of parameter "<<current_formal->get_name()<<" does not conform to declared type "<<current_formal->get_type()<<".\n";
            type = Object;
            return Object;
        }
    }

    /* obtaing the return type of the dispatch. */
    Symbol return_type = called_feature->get_return_type();

    /* if the return type is SELF_TYPE, setting it to the current class name. */
    if(return_type==SELF_TYPE)
        return_type = expr->get_expression_type(cur_class);

    type = return_type;

    return return_type;
}


/*
 * This function returns the type of the condition if-then-else expression.
 * According to the cool manual,
 * The type of the condition is evaluated first and checked if it is Bool.
 * The type of the then and else expressions are evaluated.
 * The return type of the expression is then evaluated as the join of the two expressions.
 */
Symbol cond_class::get_expression_type(Class_ cur_class)
{
    /* evaluating the type of the condition. */
    Symbol condition_type = pred->get_expression_type(cur_class);

    /* checking if the evaluated type is bool */
    if(condition_type!=Bool)
    {
        classtable->semant_error(cur_class)<<"Predicate of 'if' does not have type Bool.\n";
        type = Object;
        return Object;   
    }

    /* evaluating the type of then expression. */
    Symbol then_type = then_exp->get_expression_type(cur_class);
    if(then_type==SELF_TYPE)
        then_type = cur_class->get_name();

    /* evaluating the type of else expression. */
    Symbol else_type = else_exp->get_expression_type(cur_class);
    if(else_type==SELF_TYPE)
        else_type = cur_class->get_name();
    
    /* evaluating the join of then and else expression. */
    Symbol return_type = get_least_common_ancestor_type(then_type, else_type);
    type = return_type;
    return type;
}


/*
 * This function returns the type of loop expression.
 * According to the cool manual,
 * The type of the condition expression is evaluated first and checked if it is Bool.
 * The return type of the expression is then evaluated as the type of the body expression.
 */
Symbol loop_class::get_expression_type(Class_ cur_class)
{   
    /* evaluating the condition expression. */
    Symbol condition_type = pred->get_expression_type(cur_class);

    /* checking if the type is Bool. */
    if(condition_type!=Bool)
    {
        classtable->semant_error(cur_class)<<"Predicate of 'if' does not have type Bool.\n";
        type = Object;
        return Object;   
    }

    /* evaluating the type of the body expression. */
    Symbol body_type = body->get_expression_type(cur_class);
    type = Object;
    return Object;
}


/*
 * This function returns the type of the case expression.
 * According to the cool manual,
 * The type of the case expression is evaluted first.
 * Next, each of the branches are evaluated.
 * The type of each branch is checked if they are defined and are unique.
 * The identifier in each branch is then added to the attribute table.
 * The type of the expression of each branch is evaluated.
 * The return type is evaluated as the join of the type of expression of each branch.
 */
Symbol typcase_class::get_expression_type(Class_ cur_class)
{
    /* obtaining the inheritance graph. */
    std::map<Symbol, Class_>inheritance_graph = classtable->get_inheritance_graph();
    Symbol return_type = NULL;

    /* evaluating the case expression. */
    expr->get_expression_type(cur_class);

    /* creating a vector to store the type of each branch. */
    std::vector<Symbol> type_map;

    /* iterating over each branch. */
    for(int i=cases->first(); cases->more(i); i=cases->next(i))
    {
        Case current_case = cases->nth(i);

        /* obtaining the type of the current branch. */
        Symbol current_type = current_case->get_type();

        /* checking if current_type is defined. */
        if(inheritance_graph.find(current_type)==inheritance_graph.end())
        {
            classtable->semant_error(cur_class)<<"Class "<<current_type<<" of case branch is undefined.\n";
            type = Object;
            return Object;  
        }

        /* checking if the type is already present in the vector. */
        for(int i=0; i<type_map.size(); i++)
        {
            if(type_map[i]==current_type)
            {
                classtable->semant_error(cur_class)<<"Duplicate branch "<<current_type<<" in case statement.\n";
                type = Object;
                return Object;
            }
        }

        /* pushing the current type in the vector. */
        type_map.push_back(current_type);

        /* adding the identifier of the branch in the attribute table. */
        attribute_table->enterscope();
        attribute_table->addid(current_case->get_name(), new Symbol(current_type));

        /* evaluating the type of the branch expression and evaluating the return type as the join of the types. */
        Symbol temp_type = current_case->get_expression()->get_expression_type(cur_class);
        if(return_type==NULL)
            return_type = temp_type;
        else
            return_type = get_least_common_ancestor_type(return_type, temp_type);
        attribute_table->exitscope();
    }

    type = return_type;
    return return_type;
}


/* 
 * This function returns the type of the block expression.
 * According to cool manual,
 * The return type is the type of the last expression in the block.
 */
Symbol block_class::get_expression_type(Class_ cur_class)
{
    Symbol body_type;
    /* iterating over the expressions in the block and updating the return type. */
    for(int i=body->first(); body->more(i); i=body->next(i))
    {
        Expression current_expression = body->nth(i);
        body_type = current_expression->get_expression_type(cur_class);
    }

    type = body_type;
    return body_type;
}


/*
 * This function returns the type of the let expression.
 * According to the cool manual,
 * It first populates the attribute table with the identifies in the let statement.
 * The return type is evaluated as the type of the body of the let expression.
 */
Symbol let_class::get_expression_type(Class_ cur_class)
{
    /* identifier cant be self. */
    if(identifier==self)
    {
        classtable->semant_error(cur_class)<<"'self' cannot be bound in a 'let' expression.\n";
        return Object;
    }

    /* evaluating the type of the init expression, and checking if it conforms to the identifier. */
    if(init->get_expression_type(cur_class)!=No_type && type_decl!=init->get_expression_type(cur_class))
    {
        classtable->semant_error(cur_class)<<"Inferred type "<<init->get_expression_type(cur_class)<<" of initialization of "<<identifier<<" does not conform to identifier's declared type "<<type_decl<<".\n";
        type = Object;
        return Object;
    }

    /* populating the attribute table and evaluating the type of the body. */
    attribute_table->enterscope();  
    attribute_table->addid(identifier, new Symbol(type_decl));
    type = body->get_expression_type(cur_class);
    attribute_table->exitscope();
    return type;
}


/* 
 * This function returns the type of the plus expression.
 * According to the cool manual,
 * It first checks that the type of the expression on each side of the '+' operator is Int.
 * The return type is then set to Int.
 */
Symbol plus_class::get_expression_type(Class_ cur_class)
{
    /* checking if the type of expression on each side is Int. */
    if(e1->get_expression_type(cur_class)!=Int || e2->get_expression_type(cur_class)!=Int)
    {
        classtable->semant_error(cur_class)<<"non-Int arguments: "<<e1->get_expression_type(cur_class)<<" + "<<e2->get_expression_type(cur_class)<<".\n";
        type = Object;
        return Object;
    }

    type = Int;
    return Int;
}


/* 
 * This function returns the type of the sub expression.
 * According to the cool manual,
 * It first checks that the type of the expression on each side of the '-' operator is Int.
 * The return type is then set to Int.
 */
Symbol sub_class::get_expression_type(Class_ cur_class)
{

    /* checking if the type of expression on each side is Int. */
    if(e1->get_expression_type(cur_class)!=Int || e2->get_expression_type(cur_class)!=Int)
    {
        classtable->semant_error(cur_class)<<"non-Int arguments: "<<e1->get_expression_type(cur_class)<<" - "<<e2->get_expression_type(cur_class)<<".\n";
        type = Object;
        return Object;
    }

    type = Int;
    return Int;
}


/* 
 * This function returns the type of the mul expression.
 * According to the cool manual,
 * It first checks that the type of the expression on each side of the '*' operator is Int.
 * The return type is then set to Int.
 */
Symbol mul_class::get_expression_type(Class_ cur_class)
{
    /* checking if the type of expression on each side is Int. */
    if(e1->get_expression_type(cur_class)!=Int || e2->get_expression_type(cur_class)!=Int)
    {
        classtable->semant_error(cur_class)<<"non-Int arguments: "<<e1->get_expression_type(cur_class)<<" * "<<e2->get_expression_type(cur_class)<<".\n";
        type = Object;
        return Object;
    }

    type = Int;
    return Int;
}


/* 
 * This function returns the type of the divide expression.
 * According to the cool manual,
 * It first checks that the type of the expression on each side of the '/' operator is Int.
 * The return type is then set to Int.
 */
Symbol divide_class::get_expression_type(Class_ cur_class)
{
    /* checking if the type of expression on each side is Int. */
    if(e1->get_expression_type(cur_class)!=Int || e2->get_expression_type(cur_class)!=Int)
    {
        classtable->semant_error(cur_class)<<"non-Int arguments: "<<e1->get_expression_type(cur_class)<<" / "<<e2->get_expression_type(cur_class)<<".\n";
        type = Object;
        return Object;
    }

    type = Int;
    return Int;
}


/* 
 * This function returns the type of the neg expression.
 * According to the cool manual,
 * It first checks that the type of the expression in neg expression is Int.
 * The return type is then set to Int.
 */
Symbol neg_class::get_expression_type(Class_ cur_class)
{
    /* checking that the type of the expression is Int. */
    if(e1->get_expression_type(cur_class)!=Int)
    {
        classtable->semant_error(cur_class)<<"Argument of '~' has type "<<e1->get_expression_type(cur_class)<<" instead of Int.\n";
        type = Object;
        return Object;
    }

    type = Int;
    return Int;
}


/* 
 * This function returns the type of the lt expression.
 * According to the cool manual,
 * It first checks that the type of the expression on each side of the expression is Int.
 * The return type is then set to Bool.
 */
Symbol lt_class::get_expression_type(Class_ cur_class)
{
    /* checking that the type of each subexpression is Int. */
    if(e1->get_expression_type(cur_class)!=Int || e2->get_expression_type(cur_class)!=Int)
    {
        classtable->semant_error(cur_class)<<"non-Int arguments: "<<e1->get_expression_type(cur_class)<<" < "<<e2->get_expression_type(cur_class)<<".\n";
        type = Object;
        return Object;
    }

    type = Bool;
    return Bool;
}


/* 
 * This function returns the type of the eq expression.
 * According to the cool manual,
 * If the subexpressions are of basic types Int, Bool or Str, the type of the subexpressions should match.
 * The return type is then set to Bool.
 */
Symbol eq_class::get_expression_type(Class_ cur_class)
{
    /* checking if the subexpressions are of basic types Int, Bool or Str. */
    Symbol first_type = e1->get_expression_type(cur_class), second_type = e2->get_expression_type(cur_class);
    if(first_type==Int || first_type==Bool || first_type==Str || second_type==Int || second_type==Bool || second_type==Str)
    {
        /* checking that the types match. */
        if(first_type!=second_type)
        {
            classtable->semant_error(cur_class)<<"Illegal comparison with a basic type.\n";
            type = Object;
            return Object;
        }
    }

    type = Bool;
    return Bool;
}


/* 
 * This function returns the type of the leq expression.
 * According to the cool manual,
 * It first checks that the type of the sub expressions are Int.
 * The return type is then set to Bool.
 */
Symbol leq_class::get_expression_type(Class_ cur_class)
{
    /* checking if the type of the subexpressions is Int. */
    if(e1->get_expression_type(cur_class)!=Int || e2->get_expression_type(cur_class)!=Int)
    {
        classtable->semant_error(cur_class)<<"non-Int arguments: "<<e1->get_expression_type(cur_class)<<" <= "<<e2->get_expression_type(cur_class)<<".\n";
        type = Object;
        return Object;
    }

    type = Bool;
    return Bool;
}


/* 
 * This function returns the type of the Not expression.
 * According to the cool manual,
 * It first checks that the type of the sub expression in comp expression is Bool.
 * The return type is then set to Bool.
 */
Symbol comp_class::get_expression_type(Class_ cur_class)
{
    /* evaluationg the type of the sub expression. */
    if(e1->get_expression_type(cur_class)!=Bool)
    {
        classtable->semant_error(cur_class)<<"Argument of 'not' has type "<<e1->get_expression_type(cur_class)<<" instead of Bool.\n";
        type = Object;
        return Object;
    }

    type = Bool;
    return Bool;
}


/* 
 * This function returns the type of the int constant as Int.
 */
Symbol int_const_class::get_expression_type(Class_ cur_class)
{
    type = Int;
    return Int;
}


/* 
 * This function returns the type of the bool constant as Bool.
 */
Symbol bool_const_class::get_expression_type(Class_ cur_class)
{
    type = Bool;
    return Bool;
}


/* 
 * This function returns the type of the string constant as Str.
 */
Symbol string_const_class::get_expression_type(Class_ cur_class)
{
    type = Str;
    return Str;
}


/*
 * This function returns the type of new expression.
 * According to cool manual,
 * It first checks the type_name of new expression is defined.
 * It then returns the type_name as the type of the new expression.
 */
Symbol new__class::get_expression_type(Class_ cur_class)
{
    /* obtaining the inheritance graph. */
    std::map<Symbol, Class_>inheritance_graph = classtable->get_inheritance_graph();

    /* obtaining the type name. */
    Symbol new_type = type_name;
    if(new_type==SELF_TYPE)
        new_type = cur_class->get_name();

    /* checking if the type name is defined. */
    if(inheritance_graph.find(new_type)==inheritance_graph.end())
    {
        classtable->semant_error(cur_class)<<"'new' used with undefined class "<<new_type<<".\n";
        type = Object;
        return Object;
    }

    type = type_name;
    return type_name;
}


/* 
 * This function returns the type of the isvoid expression as Bool.
 */
Symbol isvoid_class::get_expression_type(Class_ cur_class)
{
    e1->get_expression_type(cur_class);
    type = Bool;
    return Bool;
}


/* 
 * This function returns the type of the no_expr expression as No_type.
 */
Symbol no_expr_class::get_expression_type(Class_ cur_class)
{
    type = No_type;
    return No_type;
}


/* 
 * This function returns the type of the object expression as its class name.
 */
Symbol object_class::get_expression_type(Class_ cur_class)
{
    /* if the object is self. */
    if(name==self)
    {
        type = SELF_TYPE;
        return SELF_TYPE;
    }

    /* checking if the object is present in the scope. */
    if(attribute_table->lookup(name)==NULL)
    {
        classtable->semant_error(cur_class)<<"Undeclared identifier "<<name<<".\n";
        type = Object;
        return Object;
    }   

    /* returning the type of the associated class. */
    type = *(attribute_table->lookup(name));
    return type;
}


/*
 * This function checks a method for semantic errors.
 * It first checks the formal list of the method.
 * It then evaluates the body of the method.
 * It then checks the return type of the method.
 */
void method_class::check_feature(Class_ cur_class)
{
    /* obtainig the inheritance graph. */
    std::map<Symbol, Class_>inheritance_graph = classtable->get_inheritance_graph();
    bool is_error = false;

    /* obtaining the formals of the method. */
    Formals formals = get_formals();

    /* iterating over the formals. */
    for(int i=formals->first(); formals->more(i); i=formals->next(i))
    {
        Formal current_formal = formals->nth(i);

        /* checking if the identifier name is not self. */
        if(current_formal->get_name()==self)
        {
            classtable->semant_error(cur_class)<<"'self' cannot be the name of a formal parameter.\n";
            is_error = true;
        }

        /* checking if the identifier is not redefined. */
        if(attribute_table->probe(current_formal->get_name())!=NULL)
        {
            classtable->semant_error(cur_class)<<"Formal parameter "<<current_formal->get_name()<<" is multiply defined.\n";
            is_error = true;
        }

        /* checking if the type of the identifier is defined. */
        if(inheritance_graph.find(current_formal->get_type())==inheritance_graph.end())
        {
            classtable->semant_error(cur_class)<<"Class "<<current_formal->get_type()<<" of formal parameter "<<current_formal->get_name()<<" is undefined.\n";
            is_error = true; 
        }

        if(!is_error)
            attribute_table->addid(current_formal->get_name(), new Symbol(current_formal->get_type()));
    }

    /* evaluating the type of the body of the feature. */
    Symbol body_type = expr->get_expression_type(cur_class);
    
    /* checking the return type. */
    Symbol method_return_type = return_type;

    if(method_return_type==SELF_TYPE)
        method_return_type = cur_class->get_name();

    if(body_type==SELF_TYPE)
        body_type = cur_class->get_name();

    /* checking if the type of the body conforms to the return type. */
    if(!(return_type==body_type || check_ancestor(body_type, method_return_type)))
    {
        classtable->semant_error(cur_class)<<"Inferred return type "<<body_type<<" of method a does not conform to declared return type "<<return_type<<".\n";
        return;
    }

    if(return_type==SELF_TYPE && expr->get_expression_type(cur_class)!=SELF_TYPE)
    {
        classtable->semant_error(cur_class)<<"Inferred return type "<<expr->get_expression_type(cur_class)<<" of method a does not conform to declared return type "<<return_type<<".\n";
        return;
    }
}


/* 
 * This function checks the attribute for semantice errors.
 */
void attr_class::check_feature(Class_ cur_class)
{
    /* evaluating the type of initialization expression. */
    Symbol assigned_type = init->get_expression_type(cur_class);
    if(assigned_type==SELF_TYPE)
        assigned_type = cur_class->get_name();

    /* checking if the type conforms to the declared type. */
    if(!(assigned_type==No_type || assigned_type==type_decl || check_ancestor(assigned_type, type_decl)))
    {
        classtable->semant_error(cur_class)<<"Inferred type "<<assigned_type<<" of initialization of attribute "<<name<<" does not conform to declared type "<<type_decl<<".\n";
        return;
    }
}


/*
 * This function adds a method to the corresponding symbol table.
 */
void method_class::add_to_symbol_table(Feature current_feature, Class_ cur_class)
{
    /* flag for error. */
    bool is_error=false;

    
    if(function_table->probe(name)!=NULL)
    {
        classtable->semant_error(cur_class)<<"Method "<<name<<" is multiply defined.\n";
        return;
    }

    if(function_table->lookup(name)!= NULL)
    {
        Feature inherited_feature = *(function_table->lookup(name));
        Formals inherited_formals = inherited_feature->get_formals();

        if(formals->len()!=inherited_formals->len())
        {
            classtable->semant_error(cur_class)<<"Incompatible number of formal parameters in redefined method "<<name<<".\n";
            return;  
        }

        for(int i=formals->first(); formals->more(i); i=formals->next(i))
        {
            Formal current_formal = formals->nth(i);
            Formal inherited_formal = inherited_formals->nth(i);

            if(current_formal->get_type()!=inherited_formal->get_type())
            {
                classtable->semant_error(cur_class)<<"In redefined method "<<name<<", parameter type "<<current_formal->get_type()<<" is different from original type "<<inherited_formal->get_type()<<".\n";
                is_error=true;
                break;
            }
        }

        if(return_type!=inherited_feature->get_return_type())
        {
            classtable->semant_error(cur_class)<<"In redefined method "<<name<<", return type "<<return_type<<" is different from original return type "<<inherited_feature->get_return_type()<<".\n";
            return;
        }
    }
    if(!is_error)
        function_table->addid(name, new Feature(current_feature));

}

void attr_class::add_to_symbol_table(Feature current_feature, Class_ cur_class)
{
    if(attribute_table->probe(name)!=NULL)
    {
        classtable->semant_error(cur_class)<<"Attribute "<<name<<" is multiply defined in class.\n";
        return;
    }

    if(attribute_table->lookup(name)!=NULL)
    {
        classtable->semant_error(cur_class)<<"Attribute "<<name<<" is an attribute of an inherited class.\n";
    }

    if(name==self)
    {
        classtable->semant_error(cur_class)<<"'self' cannot be the name of an attribute.\n";
        return;
    }

    attribute_table->addid(name, new Symbol(type_decl));
}

void populate_symbol_tables(Class_ cur_class)
{
    Symbol parent = cur_class->get_parent();
    if(parent!=No_class)
    {
        populate_symbol_tables(classtable->get_inheritance_graph().find(parent)->second);
    }

    attribute_table->enterscope();
    function_table->enterscope();

    Features features = cur_class->get_features();

    for(int i=features->first(); features->more(i); i=features->next(i))
    {
        Feature feature = features->nth(i);
        feature->add_to_symbol_table(features->nth(i), cur_class);
    }
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    classtable = new ClassTable(classes);

    if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
    }
    /* some semantic analysis code may go here */
    for(int i=classes->first(); classes->more(i); i=classes->next(i))
    {
        /* getting the current class. */
        Class_ cur_class = classes->nth(i);
        function_table = new SymbolTable<Symbol, Feature>();
        attribute_table = new SymbolTable<Symbol, Symbol>();
        populate_symbol_tables(cur_class);

        Features features = cur_class->get_features();
        for(int i=features->first(); features->more(i); i=features->next(i))
        {
            Feature feature = features->nth(i);

            attribute_table->enterscope();
            function_table->enterscope();

            feature->check_feature(cur_class);

            attribute_table->exitscope();
            function_table->exitscope();

        }
    }

    if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
    }
}

# cpj - command line tool and scripting language for workflow automation

I use a simple project structure for most of my projects.
Unfortunately, it can be a bit frustrating putting files in their proper directories and updating large "hub" files.
I wanted a tool to automate all this.

cpj is a small scripting language designed to make creating automation tools for your project simpler.

# Getting Started
Build and install cpj somewhere in your path.

In the root directory of your project, run `cpj init`.
This will create a `.cpj` folder in your current path along with a `script.cpj`.
Open the script using `` nano `cpj script` `` (feel free to use your text editor of your choice).

Once you've made your script, you can invoke an action using `cpj <action_name> [arg1] [arg2] ...`.

# Creating Templates
Templates are project wide text files that are used a lot in serialization and parsing.

To create a template, use `cpj template <name>`.
This will create a template of the same name inside your `.cpj` folder.
Feel free to fill in your templates with whatever you want (presumably some basic source).

To define an insert point in your template, use the `{{ <insert_name> }}` syntax.
You can escape the mustache braces by using three mustache braces at once.

Example Template
```
#include <{{ module }}/{{ name }}>

{{ class_name }}::{{ class_name }}(Context *parent) : Context(parent, {{ kind_name }}) {
    fmt::format("Hello {{{ World! }}}");
}
```

# Language Syntax
This language syntax is used in the script.cpj file and will be used to define actions.
The language is designed to be very minimal.

## Defining Actions
Actions are the equivalent to functions in other languages.
Each "action" you define represents a command line action you can invoke using `cpj <action_name> [arg1] [arg2] ...`.

You can define one by writing its name in global scope (not within another action) followed by the name of each argument.

Example (defines action called `create`, with arguments `name` and `type`)
```
create name type { }
```

Statements can be put into between the `{ }` on separate lines to define behaviour.

Optionally, the last argument can be proceeded with `...` to capture the concatenated values of every argument after it.

## String Literals
Use single quotes to define a string literal.
You can use `\'` to escape a quote, `\n` `\t` and `\\` declare a newline, tab and backslash respectively.
You can use `${expression}` within a string literal to insert the value of an expression at that point.
Dollar signs can be escaped with `\$`.

Example
```
'Hello World! \n Status: ${log_status}'
```

## Declaring Variables
Generally you can define or set a variable by postfixing its name with a `=` followed by its value.
Its value is generally an expression, meaning it can be the result of an action, string literal or another variable.

Example (declares a variable with name 'num_pizza_flavors' that's initialized to the result of `size pizza_flavors`).
```
num_pizza_flavors = size pizza_flavors
```

## Fetching Variables and Invoking Actions
You can fetch the value of a variable to use elsewhere by typing the name of the variable alone.
You can invoke an action by typing the name of the action followed by its parameters.
For example `size pizza_flavors` calls the function size with the argument pizza_flavors.

Some builtin functions include:

Action Name | Function
--- | ---
`template <name>` | Evaluates to the contents of the template with the name `<name>`.
`read <path>` | Evaluates to the contents of the file at `<path>`. Relative paths can be used to get files in the current directory.
`write <data> <path>` | Writes `<data>` to the contents of the file at `<path>`. Relative paths can be used to get files in the current directory.
`directory <path>` | Ensures directories in `<path>` are created.
`print <value>` | Prints `<value>` to the console.
`substr <text> <start> <count>` | Takes the substring of text at index `<start>` for `<count>` characters.
`size <text>` | Evaluates to the size of `<text>` in characters.
`lowercase <text>` | Evaluates to `<text>` where every letter has been transformed to lowercase.
`uppercase <text>` | Evaluates to `<text>` where every letter has been transformed to uppercase.

And the following operators: `a`, `s`, `m`, `d`, `n`, `e`, `ne`, `g`, `ge`, `l`, `le`, `or`, `and`, `t`.

If an action invoke is being passed as a parameter to another action, it must be wrapped in `()` to prevent ambiguity.
When a newline is hit, cpj will assume the last parameter has been passed.

Example (write template `my_template.cpp` to the path `DIR/NEW_FILE.CPP`)
```
directory 'DIR'
path = uppercase 'dir/new_file.cpp'
write (template 'my_template.cpp') path
```

## Template Expressions
A template expression can be created by proceeding a variable name or action invoke (in braces) by `{ }`.
They are used to insert values at inserts defined in a template.

JSON style syntax is used within the braces (`'insert_name': value`) on separate lines and without commas.

Example
```
name = 'peperoni'

filled_in_template = (template `source.cpp`) {
  'module': 'pizza',
  'flavor': name
}
```

## Parse Expressions
The parse expression undoes a template expression.
It is generally used to parse existing files to manipulate the current values at the insert points.
A parse expression is invoked like a template expression with some extra syntax.
Generally, the syntax is `content -> content_template { 'insert_name': output_variable_name }`.

Example (prints `absolutely`)
```
file = 'fish: yes, swimming: absolutely, taste: no'
object = `fish: {{ a }}, swimming: {{ b }}, taste: {{ c }}`

file -> object {
  'fish': x
  'swimming': y
}

print 'Is it a fish? ${x} Is it swimming: ${y}'
```

## Ifs and Fors
Ifs and for statements can be defined as follows:
```
if expression {
  statements...
}

for expression {
  statements...
}
```

The if statement will only execute the inner statements if the expression evaluates to `'true'`.
The for statement will continually execute the inner statements until the expression does not evalutate to `'true'`.

## Using Is
Use an `is` statement to instantly return from the current action and return a value.

Example
```
uppercase_concat value1 value2 {
  is uppercase '${value1}${value2}'
}
```

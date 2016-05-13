# Generics

Generics are a form of metaprogramming available in Acorn. They work by saying that a type of a method or another type
can be many things. A concrete version of that method or type is built at compile-time depending on usage.

When you try to identify a type or a function with type parameters, i.e. `Array{Integer}` or `max{Integer}` you are
telling the compile to ensure a version of the generic type or method with those types are available during
compile-time. Technically, this means you only need to specify the type parameters once (especially for methods where
multiple dispatch selects the appropriate method) but it is often best to specify this in all uses. Note that this
may change in the future with appropriate type inference.

During compile-time, the type parameters are erased from the identifiers, but this is an implementation detail and
should not factor in your programming design.

# TODO

## ObjectAlias parent views

Consider adding `ObjectAlias:as(parentClass)` for explicit access to a parent
implementation or a hidden parent field. The method should validate that the
requested class belongs to the alias type's ancestry and return a view using the
same base address. This is intentionally deferred from the structural class-cache
refactor; basic `Class` representations do not need a corresponding `:as` API.

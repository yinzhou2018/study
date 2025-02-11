#include "reflection_macros.inl"

BEGIN_REFLECTION_TYPE(ReflectionObject, reflection::Object)
BEGIN_REFLECTION_PROPERTY(ReflectionObject)
REFLECTION_PROPERTY(ReflectionObject, age, reflection::IntObject)
REFLECTION_PROPERTY(ReflectionObject, name, reflection::StringObject)
END_REFLECTION_PROPERTY()
BEGIN_REFLECTION_METHOD(ReflectionObject)
REFLECTION_METHOD_2(ReflectionObject, add_age, reflection::IntObject, reflection::IntObject, reflection::IntObject)
REFLECTION_METHOD_1(ReflectionObject, reanme, reflection::StringObject, reflection::StringObject)
END_REFLECTION_METHOD()
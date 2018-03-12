#include <string>
#include <pybind11/pybind11.h>

namespace py = pybind11;

int add(int i, int j) {
    return i + j;
}

int add_def(int i = 0, int j = 0) {
    return i + j;
}

struct Pet {

    enum Kind {
        Dog = 0,
        Cat
    };

    Pet(const std::string &name, const std::string &owner = "") :
        name(name), owner(owner) { }
    Pet(const std::string &name, Kind type, const std::string &owner = "") :
        name(name), type(type), owner(owner) { }
    void setName(const std::string &name_) { name = name_; }
    const std::string &getName() const { return name; }

    void set(int age_) { age = age_; }
    void set(const std::string &name_) { name = name_; }

    std::string name;
    std::string owner;
    Kind type;
    int age;
};

struct Dog : Pet {
    Dog(const std::string &name) : Pet(name) { }
    std::string bark() const { return "woof!"; }
};

struct PolymorphicPet {
    virtual ~PolymorphicPet() = default;
};

struct PolymorphicDog : PolymorphicPet {
    const std::string bark() const { return "woof!"; }
};

PYBIND11_MODULE(example, m) {

// Specify module docstring.

    m.doc() = "An example module.";

// Register function w/ docstring.

    m.def("add1", &add, "A function to add two integers.");

// Name parameters of a function.

    m.def("add2", &add, "A function to add two integers.",
                        py::arg("i"), py::arg("j"));

// This will show arg names in usage info as follows:
// 
//     >>> help(example)
//     Help on module example:
// 
//     NAME
//         example - example module
// 
//     FUNCTIONS
//         add1(...) method of builtins.PyCapsule instance
//             add1(arg0: int, arg1: int) -> int
// 
//             A function to add two integers
// 
//         add2(...) method of builtins.PyCapsule instance
//             add2(i: int, j: int) -> int
// 
//             A function to add two integers
// 
//     FILE
//         /home/mg6/lab/pybind11/example.cpython-36m-x86_64-linux-gnu.so

// There are 2 possible ways to name args:

    m.def("add3", &add, py::arg("i"), py::arg("j"));

// or with pybind11's string literals:

    using namespace pybind11::literals;
    m.def("add4", &add, "i"_a, "j"_a);

// Default parameters need to be specified when registering functions.

    m.def("add5", &add_def, "A function to add two integers with def. params.",
          py::arg("i") = 0, py::arg("j") = 0);
    m.def("add6", &add_def, "A function to add two integers with def. params.",
          "i"_a = 0, "j"_a = 0);

// Variables can be exported as well. `py::cast` comes in handy to cast values.

    m.attr("the_answer") = 42;
    py::object world = py::cast("World");
    m.attr("what") = world;

// Bind class- or struct-style data structures with py::class_<C>.
// Allow dynamic object attributes (from __dict__) with py::dynamic_attr.

    py::class_<Pet> pet(m, "Pet", py::dynamic_attr());

// Constructor definition: parameter type & name:
    pet .def(py::init<const std::string &>(), "name"_a)
        .def("setName", &Pet::setName, "Set pet name.", "name"_a)
        .def("getName", &Pet::getName, "Get pet name.")

// Add __repr__ method for inspection:

        .def("__repr__", [](const Pet &o) {
            return "<example.Pet named '" + o.name + "' owned by '" + o.owner + "'>";
        }, "Return repr(self).")

// Expose a property with read/write possibility. There is also def_readonly.
//
// >>> pet = example.Pet("Rocky")
// >>> pet.owner = "someone"
// >>> print(pet.owner)

        .def_readwrite("owner", &Pet::owner, "Owner name.")

// or with specified getter & setter:

        .def_property("name", &Pet::getName, &Pet::setName, "Pet name.");

// In case of inheritance between 2 classes, there are following possibilities:

    py::class_<Dog, Pet>(m, "Dog")
        .def(py::init<const std::string &>(), "name"_a)
        .def("bark", &Dog::bark, "Bark like a dog.");

// or equivalently:

    // py::class_<Dog>(m, "Dog", pet, py::dynamic_attr())
    //     .def(py::init<const std::string &>(), "name"_a)
    //     .def("bark", &Dog::bark, "Bark like a dog.");

// It is important to note that for non-virtual class objects behind base class
// pointer, there is no automatic upcasting. So assuming:

    m.def("pet_store", []() {
      return std::unique_ptr<Pet>(new Dog("Rocky"));
    });

// the following code:
//
//    >>> p = example.pet_store()
//    >>> type(p)
//    <class 'example.Pet'>
//    >>> p.bark()
//    Traceback (most recent call last):
//      File "<stdin>", line 1, in <module>
//    AttributeError: 'example.Pet' object has no attribute 'bark'
//
// returns a Dog instance, yet Python sees only a Pet instance.
// The class should be marked virtual to resolve this problem.

    py::class_<PolymorphicPet>(m, "PolymorphicPet");
    py::class_<PolymorphicDog, PolymorphicPet>(m, "PolymorphicDog")
        .def(py::init<>())
        .def("bark", &PolymorphicDog::bark);

    m.def("pet_store2", []() {
      return std::unique_ptr<PolymorphicPet>(new PolymorphicDog());
    });

// Now, the derived class' methods are correctly called:
//
//    >>> p = example.pet_store2()
//    >>> type(p)
//    <class 'example.PolymorphicDog'>    # notice concrete resolved type
//    >>> p.bark()
//    'woof!'

// It is possible to register overloaded functions:

    // pet.def("set", (void (Pet::*)(int)) &Pet::set, "Set the pet's age")
    //    .def("set", (void (Pet::*)(const std::string &)) &Pet::set,
    //         "Set the pet's name");
// or using C++14 compatible compiler (MSVC 2015+):
    pet.def("set", py::overload_cast<int>(&Pet::set),
            "Set the pet's age", "age"_a)
       .def("set", py::overload_cast<const std::string &>(&Pet::set),
            "Set the pet's name", "name"_a);

// Enumerations need to be specified in the context of enclosing class.

    pet.def(py::init<const std::string &, Pet::Kind>(), "name"_a, "kind"_a);

    py::enum_<Pet::Kind>(pet, "Kind")
        .value("Dog", Pet::Kind::Dog)
        .value("Cat", Pet::Kind::Cat)
        .export_values();

// There exists py::arithmetic() tag that will enable arithmetic and bit-level
// operations on enum's values.
//
//  py::enum_<Pet::Kind>(pet, "Kind", py::arithmetic());
}

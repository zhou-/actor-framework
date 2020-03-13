.. _message:

Messages and Message Views
==========================

Messages in CAF are stored as type-erased, fixed-length, copy-on-write tuples.
Usually, actors decompose incoming messages automatically by pattern-matching
the message content to the message handlers.

However, some use cases (for example when implementing default handlers) require
users to interact with a ``message`` directly.

.. _type-id:

Type IDs: Adding Custom Message Types
-------------------------------------

CAF assigns each type an unsigned 16-bit *type ID*. Actors cannot send types
without such an ID.

Type IDs are assigned at compile time by specializing ``type_id``. More
specifically, CAF expects each type to specialize these templates:

.. code-block:: C++

  /// Maps the type `T` to a globally unique ID.
  template <class T>
  struct type_id;

  /// Maps the type `T` to a human-readable name.
  template <class T>
  struct type_name;

  /// Maps the globally unique ID `V` to a type (inverse to ::type_id).
  /// @relates type_id
  template <type_id_t V>
  struct type_by_id;

Specializing these templates and assigning IDs is automated by the two macros
``CAF_BEGIN_TYPE_ID_BLOCK`` and ``CAF_ADD_TYPE_ID`` by listing custom types in a
*type ID block*.

The types within a named *type ID block* receive ascending IDs, starting at the
given first ID. In the following example, we forward-declare the types ``foo``
and ``foo2`` and register them to CAF in a *type ID block*. The name of the type
ID block is arbitrary, but it must form a valid C++ identifier.

.. literalinclude:: /examples/custom_type/custom_types_1.cpp
   :language: C++
   :start-after: --(rst-type-id-block-begin)--
   :end-before: --(rst-type-id-block-end)--

The *type ID* block equips CAF with all type information required *at
compile-time*. However, CAF also needs to be able to map type IDs to meta type
information *at runtime* in order to deserialize messages it receives via the
network.

Initializing the runtime-type information, users are required to call:

.. code-block:: C++

  init_global_meta_objects<${TypeIdBlock}_type_ids>();

Whereas ``${TypeIdBlock}`` is the name of the type ID block containing the
user-defined types. In our example above, we would have to call:

.. code-block:: C++

  init_global_meta_objects<custom_types_1_type_ids>();

When calling this function, the current compilation unit must have access to
*all* ``inspect`` overloads for the listed types (see :ref:`type-inspection`).

As the name suggests, the function ``init_global_meta_objects`` sets *global
state*. This call is *not* thread safe and an application **must** call this
function *before* starting an ``actor_system``.

Class ``message``
-----------------

+---------------------------+--------------------------------------------------+
| **Observers**                                                                |
+---------------------------+--------------------------------------------------+
| ``types``                 | Returns the type IDs of all elements.            |
+---------------------------+--------------------------------------------------+
| ``size``                  | Returns the size of this message.                |
+---------------------------+--------------------------------------------------+
| ``empty``                 | Returns ``size() == 0``.                         |
+---------------------------+--------------------------------------------------+
| ``operator bool``         | Returns whether this message contains any data.  |
+---------------------------+--------------------------------------------------+
| ``operator!``             | Returns the inverse of ``operator bool``.        |
+---------------------------+--------------------------------------------------+
| ``save``                  | Serialiyes the content of this message.          |
+---------------------------+--------------------------------------------------+
| ``type_at``               | Returns the type ID of a single element.         |
+---------------------------+--------------------------------------------------+
| ``match_element<T>``      | Returns `type_id_v<T> == type_at(index)`.        |
+---------------------------+--------------------------------------------------+
| ``get_as<T>``             | Returns a string representation of the tuple.    |
+---------------------------+--------------------------------------------------+
| ``match_elements<Ts...>`` | Returns `types() == make_type_id_list<Ts...>()`. |
+---------------------------+--------------------------------------------------+
| **Modifiers**                                                                |
+---------------------------+--------------------------------------------------+
| ``swap``                  | Exchanges the content of `*this` and `other`.    |
+---------------------------+--------------------------------------------------+
| ``get_mutable_as<T>``     | Returns a mutable reference to the nth element.  |
+---------------------------+--------------------------------------------------+
| ``load``                  | Writes the tuple to ``x``.                       |
+---------------------------+--------------------------------------------------+

Class ``message_builder``
-------------------------

+---------------------------------------------------+-----------------------------------------------------+
| **Observers**                                     |                                                     |
+---------------------------------------------------+-----------------------------------------------------+
| ``bool empty()``                                  | Returns whether this message is empty.              |
+---------------------------------------------------+-----------------------------------------------------+
| ``size_t size()``                                 | Returns the size of this message.                   |
+---------------------------------------------------+-----------------------------------------------------+
| ``message to_message(	)``                         | Converts the buffer to an actual message object.    |
+---------------------------------------------------+-----------------------------------------------------+
| ``append(T val)``                                 | Adds ``val`` to the buffer.                         |
+---------------------------------------------------+-----------------------------------------------------+
| ``append(Iter first, Iter last)``                 | Adds all elements from range ``[first, last)``.     |
+---------------------------------------------------+-----------------------------------------------------+
| ``message extract(message_handler)``              | See extract_.                                       |
+---------------------------------------------------+-----------------------------------------------------+
| ``message extract_opts(...)``                     | See extract-opts_.                                  |
+---------------------------------------------------+-----------------------------------------------------+
|                                                   |                                                     |
+---------------------------------------------------+-----------------------------------------------------+
| **Modifiers**                                     |                                                     |
+---------------------------------------------------+-----------------------------------------------------+
| ``optional<message>`` ``apply(message_handler f)``| Returns ``f(*this)``.                               |
+---------------------------------------------------+-----------------------------------------------------+
| ``message move_to_message()``                     | Transfers ownership of its data to the new message. |
+---------------------------------------------------+-----------------------------------------------------+

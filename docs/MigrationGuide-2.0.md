# Migrating to glTF-SDK 2.0

## ExtrasDocument

The public JSON DOM and `GetDocument()` were removed.

Before:

```cpp
const auto json = Serialize(extras.GetDocument());
const bool hasName = extras.GetDocument().HasMember("name");
```

After:

```cpp
const auto json = extras.ToJson();
const bool hasName = extras.HasMember("name");
```

Typed operations remain available for `bool`, signed/unsigned 32- and 64-bit
integers, `size_t`, `float`, `double`, and `std::string`:

```cpp
ExtrasDocument extras;
extras.SetMemberValue("count", std::uint32_t{3});
extras.SetPointerValue("/metadata/enabled", true);

const auto count =
    extras.GetMemberValueOrDefault<std::uint32_t>("count");
const auto enabled =
    extras.GetPointerValueOrDefault<bool>("/metadata/enabled");
```

`ExtrasDocument` is move-only. A moved-from object returns defaults/`null` and
can be assigned again. Assignments preserve the JSON compatibility category:
all numbers are compatible with numbers, but a number cannot later become a
boolean or string.

## Schema validation

Pass serialized JSON rather than a vendor document:

```cpp
ValidateDocumentAgainstSchema(
    documentJson,
    schemaUri,
    std::move(schemaLocator));
```

`ISchemaLocator`, `SchemaFlags`, and `GetDefaultSchemaLocator` are unchanged.
Locator content is copied immediately. Relative and parent references,
fragments, Draft-04 `id` scopes, repeated references, and legal recursion are
resolved per validation call.

## Extensions

Extension registration APIs still use serialized JSON strings. Build
extension values with your application's JSON facilities or fixed
serialization code:

```cpp
document.extensions["EXT_example"] =
    R"({"enabled":true,"factor":0.5})";
document.extensionsUsed.insert("EXT_example");
```

Do not include `GLTFSDK/RapidJsonUtils.h`; it no longer exists. The SDK does
not add a replacement public generic JSON builder because that would recreate
the vendor boundary removed in 2.0.

Extension and extras strings are parsed strictly. Duplicate names, invalid
UTF-8, malformed JSON, and values deeper than 256 containers are rejected.

## BOM and strict-input changes

If legacy content starts with a UTF-8 BOM, pass
`DeserializeFlags::IgnoreByteOrderMark`. Without the flag, both string and
stream overloads throw. UTF-16 and UTF-32 input are not accepted.

Inputs accepted by 1.9.5 solely because they contained duplicate names or
malformed UTF-8 must be repaired. Disabling schema validation does not disable
strict JSON parsing or defensive typed checks.

## Build and package integration

Consumers no longer need RapidJSON, nlohmann/json, or Valijson include paths,
packages, or link targets. Point includes at the installed `Inc` directory and
link the installed `GLTFSDK` library only.

The package layout includes separately named dependency licenses and
`thirdPartyNotices.txt`, but no dependency headers.

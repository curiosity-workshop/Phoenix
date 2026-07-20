# Behavior Catalog

The behavior catalog maps cockpit intent to simulator-specific bindings.

## Behavior Entry

```json
{
  "id": "lights.beacon",
  "label": "Beacon lights",
  "kind": "toggle",
  "valueType": "bool",
  "direction": "readWrite",
  "defaultUpdate": {
    "rateMs": 100,
    "bucket": 1
  },
  "bindings": {
    "xplane": {
      "default": {
        "read": {
          "dataref": "sim/cockpit/electrical/beacon_lights_on",
          "type": "int"
        },
        "write": {
          "command": "sim/lights/beacon_lights_toggle"
        }
      }
    }
  }
}
```

## Catalog Layers

Catalogs should merge in this order:

1. Built-in base catalog.
2. Simulator-specific catalog.
3. Aircraft family catalog.
4. Aircraft/version override.
5. User local override.

Later entries may override simulator bindings while preserving the same behavior
ID.

## Behavior Kinds

Initial kinds:

- `toggle`: boolean state with optional toggle/set commands.
- `momentary`: command-like behavior with start/end/trigger.
- `axis`: numeric value with range and units.
- `display`: read-only formatted value.
- `enum`: finite set of named states.
- `data`: explicit byte payload for special cases.

## Value Types

Initial value types:

- `bool`
- `int`
- `float`
- `string`
- `data`
- `enum`

Data values must always carry an explicit byte length. They must never be
treated as null-terminated strings.

## Open Questions

- Should behavior IDs be fully community-owned, or should the core project
  reserve top-level namespaces?
- Should aircraft overrides be selected manually, detected automatically, or
  both?
- Should writable toggles prefer direct writable values when available, or
  commands for simulator compatibility?

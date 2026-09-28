# ${{ values.name }}

${{ values.description }}

A custom Oracle BRM 15 Facility Module, **fm_${{ values.module }}**, created from the `brm-custom-fm` golden path in MAGI. This repository holds the code only: Nerv has no BRM installation, so nothing here is deployed to Geofront.

## Opcodes

| Opcode | Number | Input | Output |
|---|---|---|---|
| `${{ values.opcodeMacro }}` | ${{ values.opcodeNumber }} | `PIN_FLD_POID` of an `/account` | `PIN_FLD_POID`, `PIN_FLD_ACCOUNT_NO`, `PIN_FLD_STATUS` |

`${{ values.opcodeMacro }}` starts as a read-only account lookup. Replace its logic, then update this table.

## Layout

```
include/${{ values.module }}_ops.h      Opcode numbers: the public contract of the module
src/fm_${{ values.module }}_config.c    Opcode → handler table the CM loads
src/ops/                 One handler per opcode: the PCM layer, runs only inside the CM
src/logic/               Flist logic with no PCM calls, covered by unit tests
src/common/              Helpers shared by the handlers (transactions)
test/unit/               cmocka unit tests of src/logic/
test/flists/             Input flists for testnap, one folder per opcode
conf/pin.conf.d/         The pin.conf entry that loads the FM into a CM
```

## Lint, build and test

`make lint` (clang-format and cppcheck) needs no BRM, and CI runs it on every push.

Compiling and unit testing need the Oracle BRM 15 SDK, which Nerv does not provide. With an SDK image you are licensed to use:

```bash
docker build --build-arg BRM_SDK_IMAGE=<your-sdk-image> .   # make, then make lint test
```

Or, anywhere with `PIN_HOME` set to a BRM 15 SDK, run `make`, `make test` and `make lint` directly. In CI, set the `BRM_SDK_IMAGE` repository variable to turn on the `build` job, which is skipped without it.

## Try it on a BRM installation

On a BRM 15 system you have access to:

1. Copy `build/lib/fm_${{ values.module }}.so` to `$PIN_HOME/lib/`.
2. Add the line from `conf/pin.conf.d/fm_${{ values.module }}.conf` to the CM's `pin.conf` and restart the CM.
3. Call the opcode with testnap, using its number from `include/${{ values.module }}_ops.h`:

   ```
   r << XX 1
   0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 1 0
   XX
   xop ${{ values.opcodeNumber }} 0 1
   ```

Set `- cm loglevel 3` in the CM's `pin.conf` to see every input and output flist in `cm.pinlog`.

## Add an opcode

1. Pick a number that no other FM uses: search the MAGI catalog for `nerv.io/brm-opcodes`.
2. Define it in `include/${{ values.module }}_ops.h` and add it to `nerv.io/brm-opcodes` in `catalog-info.yaml`.
3. Add its logic under `src/logic/` and its unit tests under `test/unit/`.
4. Add its handler under `src/ops/`, copying the structure of `op_${{ values.module }}_${{ values.opcodeLower }}.c`.
5. Register the handler in `src/fm_${{ values.module }}_config.c`.
6. Add a sample input under `test/flists/<opcode>/` and document the opcode in the table above.

## Conventions

- **Handlers are thin.** Check the opcode, validate, open a transaction, call the logic, commit or abort. Business rules go in `src/logic/`, where they can be unit tested.
- **Check the errbuf after every call** and stop at the first error. Never return a partial output flist.
- **Free every flist on every path.** The CM is a long-running process, so small leaks add up. `GET` borrows a field, `TAKE` and `PUT` move ownership, `SET` and `COPY` copy.
- **No nested transactions.** Use `${{ values.module }}_trans_open`, which only opens one if the caller has not.
- **Read only what you need.** Use `PCM_OP_READ_FLDS` rather than `PCM_OP_READ_OBJ`, and indexed, bounded searches.
- **Go through FM opcodes to change business objects.** Writing them with base opcodes skips validation, events and the audit trail.
- **Prefix every symbol with `${{ values.module }}_`.** All FMs share one CM process.
- **Log flists at debug level only.** They can hold customer and payment data.

## CI

| Job | Runs | Needs |
|---|---|---|
| `lint` | Every pull request and push to `main` | Nothing |
| `build` | Same, only when the `BRM_SDK_IMAGE` variable is set | A BRM 15 SDK image |

There is no image and no deployment: this repository is not connected to Central Dogma.

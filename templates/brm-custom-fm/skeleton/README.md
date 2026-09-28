# ${{ values.name }}

${{ values.description }}

A custom Oracle BRM 15 Facility Module, **fm_${{ values.module }}**, created from the `brm-custom-fm` golden path in MAGI. It runs in its own Connection Manager (CM), deployed to Geofront through Central Dogma.

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
conf/pin.conf.d/         The pin.conf entry that loads the FM, baked into the image
docker/cm-entrypoint.sh  Builds the CM's pin.conf at startup
scripts/testnap.sh       Calls an opcode on the dev CM
```

## Build and test

Everything runs inside the BRM SDK image, so you only need Docker:

```bash
docker build --target test .      # clang-format, cppcheck and unit tests
docker build --target runtime .   # the CM image
```

Inside the SDK image (or anywhere with `PIN_HOME` set to a BRM 15 SDK), use `make`, `make test` and `make lint` directly.

## Call an opcode in dev

```bash
bash scripts/testnap.sh ${{ values.opcodeMacro }} test/flists/${{ values.opcodeLower }}/read_account.flist
```

Change `- cm loglevel` to 3 in Central Dogma to see every input and output flist in `cm.pinlog`.

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

## Build and deploy

- **Pull requests:** CI runs lint and unit tests in the SDK image, and builds the CM image.
- **Pushes to `main`:** CI pushes `ghcr.io/camnoss/${{ values.name }}` tagged with the commit SHA and `main`, and the Central Dogma release bot deploys it to dev.
- **Configuration:** the CM's DM connection and logging live in [Central Dogma](https://github.com/camnoss/central-dogma) under `apps/${{ values.name }}/overlays/dev/pin.conf.overrides`.
- **Credentials:** the CM reads its wallet from the `brm-cm-credentials` Secret, which the platform provides in the namespace. Until it exists, the pod waits in `ContainerCreating`.

| Environment | Namespace | ArgoCD app | CM address |
|---|---|---|---|
| dev | `${{ values.name }}-dev` | `${{ values.name }}-dev` | `${{ values.name }}.${{ values.name }}-dev.svc:11960` |

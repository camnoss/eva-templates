# ${{ values.name }}

${{ values.description }}

A customization of the Oracle BRM 15 policy opcode **`${{ values.policyOpcode }}`**, created from the `brm-policy-override` golden path in MAGI. This repository holds the code only: Nerv has no BRM installation, so nothing here is deployed to Geofront.

## How it works

Oracle ships the source of every policy library (`fm_*_pol`) with the SDK. This repository never copies it. Instead, the build:

1. Finds the stock library whose config table registers each opcode in `overrides.conf`.
2. Copies that library's source into `build/stage/`.
3. Points the opcode's config table entry at our wrapper, `nerv_op_${{ values.policy }}`.
4. Compiles the stock source and our code into one library, `build/lib/<library>.so`, which replaces the stock one.

The wrapper runs our rules, then Oracle's stock `op_${{ values.policy }}()`, then our rules on its output. Stock behavior stays in place, and an SDK upgrade only needs a rebuild. If the upgrade changes how the opcode is registered, the build fails instead of quietly running the stock policy.

## Layout

```
overrides.conf            Stock policy opcodes routed to our wrappers
src/ops/                  Wrappers: the PCM layer, runs only inside the CM
src/logic/                Our rules, with no PCM calls, covered by unit tests
test/unit/                cmocka unit tests of src/logic/
test/flists/              Input flists for testnap, one folder per opcode
scripts/route-overrides.sh  Stages the stock source and routes the opcodes
```

## Lint, build and test

`make lint` (clang-format and cppcheck) needs no BRM, and CI runs it on every push.

Compiling and unit testing need the Oracle BRM 15 SDK, including its stock policy source, which Nerv does not provide. With an SDK image you are licensed to use:

```bash
docker build --build-arg BRM_SDK_IMAGE=<your-sdk-image> .   # make, then make lint test
```

Or, anywhere with `PIN_HOME` set to a BRM 15 SDK, run `make`, `make test` and `make lint` directly. In CI, set the `BRM_SDK_IMAGE` repository variable to turn on the `build` job, which is skipped without it. If the stock source needs extra compiler flags, set `STOCK_CFLAGS` in the `Makefile` from the stock `Makefile` in `$PIN_HOME/source/sys/<library>/`.

## Try it on a BRM installation

On a BRM 15 system you have access to:

1. Back up the stock library in `$PIN_HOME/lib/`, then replace it with `build/lib/<library>.so` (the library name is in `build/stage/LIBRARY`).
2. Restart the CM. Its `pin.conf` already loads the library by name.
3. Call the opcode with testnap, with an input flist like `test/flists/${{ values.policy }}/basic.flist`:

   ```
   r << XX 1
   0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 1 0
   XX
   xop ${{ values.policyOpcode }} 0 1
   ```

Set `- cm loglevel 3` in the CM's `pin.conf` to see every input and output flist in `cm.pinlog`.

## Customize another opcode of the same library

1. Add `<PCM_OP_..._POL_...> nerv_op_<name>` to `overrides.conf` and to `nerv.io/brm-policy-opcodes` in `catalog-info.yaml`.
2. Add its rules under `src/logic/` and their unit tests under `test/unit/`.
3. Add its wrapper under `src/ops/`, copying the structure of `nerv_op_${{ values.policy }}.c`.
4. Add a sample input under `test/flists/<name>/`.

An opcode from a different policy library needs its own repository: create one from the same golden path.

## Conventions

- **Wrap, don't copy.** Call the stock function and add your rules around it. Copying Oracle's code means merging it by hand on every upgrade.
- **Wrappers are thin.** Business rules go in `src/logic/`, where they can be unit tested.
- **Report rejections as application errors** (`PIN_ERRCLASS_APPLICATION`, with the field at fault). An error from a policy opcode rolls back the caller's whole transaction.
- **Check the errbuf after every call** and free every flist on every path. The CM is a long-running process.
- **Keep it fast.** Policy opcodes can run on every event. A search here can turn into millions of queries.
- **Log flists at debug level only.** They can hold customer and payment data.

## CI

| Job | Runs | Needs |
|---|---|---|
| `lint` | Every pull request and push to `main` | Nothing |
| `build` | Same, only when the `BRM_SDK_IMAGE` variable is set | A BRM 15 SDK image |

There is no image and no deployment: this repository is not connected to Central Dogma.

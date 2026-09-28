# ${{ values.name }}

${{ values.description }}

A customization of the Oracle BRM 15 policy opcode **`${{ values.policyOpcode }}`**, created from the `brm-policy-override` golden path in MAGI. It runs in its own Connection Manager (CM), deployed to Geofront through Central Dogma.

## How it works

Oracle ships the source of every policy library (`fm_*_pol`) with the SDK. This repository never copies it. Instead, the build:

1. Finds the stock library whose config table registers each opcode in `overrides.conf`.
2. Copies that library's source into `build/stage/`.
3. Points the opcode's config table entry at our wrapper, `nerv_op_${{ values.policy }}`.
4. Compiles the stock source and our code into one library, which replaces the stock `.so` in the CM image.

The wrapper runs our rules, then Oracle's stock `op_${{ values.policy }}()`, then our rules on its output. Stock behavior stays in place, and an SDK upgrade only needs a rebuild. If the upgrade changes how the opcode is registered, the build fails instead of quietly running the stock policy.

Only calls routed to this CM run the customization. Clients that use the main CM still get the stock policy.

## Layout

```
overrides.conf            Stock policy opcodes routed to our wrappers
src/ops/                  Wrappers: the PCM layer, runs only inside the CM
src/logic/                Our rules, with no PCM calls, covered by unit tests
test/unit/                cmocka unit tests of src/logic/
test/flists/              Input flists for testnap, one folder per opcode
scripts/route-overrides.sh  Stages the stock source and routes the opcodes
docker/cm-entrypoint.sh   Builds the CM's pin.conf at startup
scripts/testnap.sh        Calls an opcode on the dev CM
```

## Build and test

Everything runs inside the BRM SDK image, so you only need Docker:

```bash
docker build --target test .      # clang-format, cppcheck and unit tests
docker build --target runtime .   # the CM image
```

Inside the SDK image (or anywhere with `PIN_HOME` set to a BRM 15 SDK), use `make`, `make test` and `make lint` directly. If the stock source needs extra compiler flags, set `STOCK_CFLAGS` in the `Makefile` from the stock `Makefile` in `$PIN_HOME/source/sys/<library>/`.

## Call the opcode in dev

```bash
bash scripts/testnap.sh ${{ values.policyOpcode }} test/flists/${{ values.policy }}/basic.flist
```

Change `- cm loglevel` to 3 in Central Dogma to see every input and output flist in `cm.pinlog`.

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

## Build and deploy

- **Pull requests:** CI runs lint and unit tests in the SDK image, and builds the CM image.
- **Pushes to `main`:** CI pushes `ghcr.io/camnoss/${{ values.name }}` tagged with the commit SHA and `main`, and the Central Dogma release bot deploys it to dev.
- **Configuration:** the CM's DM connection and logging live in [Central Dogma](https://github.com/camnoss/central-dogma) under `apps/${{ values.name }}/overlays/dev/pin.conf.overrides`.
- **Credentials:** the CM reads its wallet from the `brm-cm-credentials` Secret, which the platform provides in the namespace. Until it exists, the pod waits in `ContainerCreating`.

| Environment | Namespace | ArgoCD app | CM address |
|---|---|---|---|
| dev | `${{ values.name }}-dev` | `${{ values.name }}-dev` | `${{ values.name }}.${{ values.name }}-dev.svc:11960` |

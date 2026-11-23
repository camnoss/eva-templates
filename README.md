# Eva Templates

> Every Eva Unit starts here. Golden paths to launch new services into Geofront, from a single form in MAGI.

**Eva Templates** holds the [Backstage Software Templates](https://backstage.io/docs/features/software-templates/) of **Nerv**, an internal developer platform (IDP) built on Kubernetes, Backstage and ArgoCD.

Each template is a **golden path**: an opinionated, supported way to create a new service. With a single form in MAGI, a team gets:

- A new repository with working code, a Dockerfile and CI already in place
- A pull request in Central Dogma that deploys the service to `dev` and merges itself once the first image is built
- Automatic deploys to `dev` after every merge to `main`, through the Central Dogma release bot
- The service registered in the catalog, with its owner, Kubernetes status and ArgoCD deployments

No tickets, no copy-pasting from another repo, no waiting on the platform team.


## Where Eva Templates fits in Nerv

| Codename | Component | Repository |
|---|---|---|
| **Geofront** | Kubernetes cluster(s) | — |
| **Central Dogma** | GitOps source of truth (ArgoCD) | [`central-dogma`](https://github.com/camnoss/central-dogma) |
| **MAGI** | Developer portal (Backstage) | [`magi`](https://github.com/camnoss/magi) |
| **Eva Templates** | Golden path templates | this repository |
| **Eva Units** | Team workloads | created from these templates |

## Template conventions

- **Names** are lowercase and hyphenated, describing the stack: `go-service`, `node-service`, `static-site`.
- **Every template must produce a deployable service.** A template that stops at "repository created" is not a golden path.
- **Every generated repository must include** `catalog-info.yaml` with an `owner`, and the `backstage.io/kubernetes-id` and `argocd/app-name` annotations.
- **Every template must connect to the release bot:** run `nerv:release-bot:connect` after `publish:github`, and end CI with a `release` job that sends an `image-published` dispatch to Central Dogma (see `go-service`). CI must tag images with the full commit SHA, which is what the bot deploys.
- **Secure by default:** non-root containers, resource requests and limits, and health probes in the manifests.
- **Keep templates minimal.** Ship the smallest service that builds, deploys and passes health checks. Teams add features; templates provide the path.
- **Templates are self-contained.** Each one owns its `skeleton/` and `deploy/`, even where files look alike, so changing one never changes another.

The BRM templates and `event-state-stream` are the exceptions to the deployment rules above; see below.


## BRM templates

`brm-custom-fm` (a new Facility Module with custom opcodes) and `brm-policy-override` (a customized stock policy opcode) scaffold Oracle BRM 15 code in C.

- **Code only, no deployment.** Opcodes run inside a BRM Connection Manager, and Nerv has no BRM installation or BRM images. These templates create the repository and register it in the catalog as a `library`. They have no `deploy/`, no release bot connection and no Central Dogma pull request, and their catalog entries have no Kubernetes or ArgoCD annotations.
- **CI without BRM:** `lint` (clang-format and cppcheck) always runs. `build` compiles and runs the cmocka unit tests inside a BRM 15 SDK image, and runs only when the generated repository sets a `BRM_SDK_IMAGE` variable.
- **Build contract:** `make` (library in `build/lib/`), `make test` and `make lint`, also available through the Dockerfile with `--build-arg BRM_SDK_IMAGE=<image>`.
- **Code layout:** `src/ops/` holds thin opcode handlers, the only code that makes PCM calls. `src/logic/` holds the flist logic that unit tests cover without a CM.
- **Oracle source is never committed.** `brm-policy-override` copies the stock `fm_*_pol` source from the SDK at build time and routes the opcode to a wrapper that calls the stock implementation.
- **Licensing:** the BRM SDK is Oracle proprietary software. Don't publish it, or images built from it, on a public registry.


## Event stream template

`event-state-stream` publishes an **ECST state stream**: a compacted Kafka topic `<domain>.<entity>.state.v1` that carries the full state of one entity per key, with CloudEvents 1.0.2 headers and an Avro value. See [Event streams in Central Dogma](https://github.com/camnoss/central-dogma#event-streams).

- **No repository, no service.** The stream belongs to an existing producer, picked from the catalog. The template only renders `streams/<domain>/<entity>/` and opens a pull request in Central Dogma from `feature/stream-<domain>-<entity>`. It never uses `feature/add-*`, so the pull request is not merged automatically; the domain team reviews it.
- **The schema comes from the form.** The second page asks for the lifecycle states and the fields: name, type, whether it is required, and a description. The template adds the envelope fields (`<entity>Id`, `state` with `UNKNOWN` first, `stateVersion`, `lastModifiedAt`) and writes `schema/<Entity>State.avsc`. Optional fields are `["null", <type>]` with `default: null`. Money uses `decimal` (Avro `bytes` with precision 18, scale 2), instants `timestamp-millis` and calendar dates `date`.
- **What the folder holds:** `kafkatopic.yaml`, `kafkausers.yaml` (the producer user `<domain>-<entity>-producer`, with `Write`), the schema, `asyncapi.yaml` and `catalog-info.yaml` (an `API` linked to its producer through the `nerv.io/producer` annotation).
- **After merge** ArgoCD creates the topic and the user, CI registers the schema as `<domain>/<entity>-state`, and MAGI shows the stream as an API. From then on the registry enforces `FULL_TRANSITIVE`: new fields must be optional, and no field can be removed, renamed or change its type.

## Templating gotchas

Backstage renders files with [Nunjucks](https://mozilla.github.io/nunjucks/) using `${{ }}`, the same syntax as GitHub Actions expressions. Files that contain Actions expressions must be excluded from templating:

```yaml
- id: fetch-skeleton
  action: fetch:template
  input:
    url: ./skeleton
    copyWithoutTemplating:
      - .github/workflows/*
```

Alternatively, escape individual expressions: `${{ '${{ github.sha }}' }}`.

## Useful links

- [Backstage Software Templates](https://backstage.io/docs/features/software-templates/)
- [Built-in scaffolder actions](https://backstage.io/docs/features/software-templates/builtin-actions)
- [Central Dogma](https://github.com/camnoss/central-dogma)
- [MAGI](https://github.com/camnoss/magi)
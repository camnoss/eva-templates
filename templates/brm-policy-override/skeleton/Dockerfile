# syntax=docker/dockerfile:1

# BRM 15 SDK: gcc, make, the SDK headers, libraries and stock policy source
# under PIN_HOME, cmocka, cppcheck and clang-format.
ARG BRM_SDK_IMAGE=ghcr.io/camnoss/brm-sdk:15.0
# BRM 15 cloud-native Connection Manager image whose policy library we replace.
ARG BRM_CM_IMAGE=ghcr.io/camnoss/brm-cm:15.0

FROM ${BRM_SDK_IMAGE} AS build

WORKDIR /src
COPY . .
RUN make

# CI builds this stage on every push: `docker build --target test .`
FROM build AS test
RUN make lint test

FROM ${BRM_CM_IMAGE} AS runtime

# PIN_HOME of the CM image.
ARG PIN_HOME=/oms

# Replaces the stock policy library. pin.conf already loads it by name.
COPY --from=build /src/build/lib/ ${PIN_HOME}/lib/
COPY --chmod=0755 docker/cm-entrypoint.sh /usr/local/bin/cm-entrypoint.sh

# Numeric UID of the CM image's user, so Kubernetes can enforce runAsNonRoot.
USER 1000:1000
EXPOSE 11960

ENTRYPOINT ["/usr/local/bin/cm-entrypoint.sh"]

# syntax=docker/dockerfile:1

# Builds and tests the policy library inside a BRM 15 SDK image. Nerv does not
# provide one, because the SDK is Oracle licensed software: pass your own.
#
#   docker build --build-arg BRM_SDK_IMAGE=<your-sdk-image> .
#
# The image needs gcc, make, the SDK headers, libraries and stock policy source
# under PIN_HOME, cmocka, cppcheck and clang-format.
#
# The placeholder makes a build without it fail with a message that says why.
ARG BRM_SDK_IMAGE=set-build-arg-brm_sdk_image-to-your-brm-15-sdk-image

FROM ${BRM_SDK_IMAGE} AS build

WORKDIR /src
COPY . .
RUN make

FROM build AS test
RUN make lint test

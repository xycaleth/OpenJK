#!/usr/bin/env bash

. "${VULKAN_SDK}/setup-env.sh"

TMPDIR=$(mktemp -d)
SPVDIR="${TMPDIR}/spv"

mkdir -p "${SPVDIR}"
glslc -O                      -o "${SPVDIR}/render.vert.spv"                       render.vert
glslc -O                      -o "${SPVDIR}/render.frag.spv"                       render.frag
glslc -O -DUSE_MULTITEXTURE   -o "${SPVDIR}/render.multitexture.vert.spv"          render.vert
glslc -O -DUSE_MULTITEXTURE=1 -o "${SPVDIR}/render.multitexture.add.frag.spv"      render.frag
glslc -O -DUSE_MULTITEXTURE=2 -o "${SPVDIR}/render.multitexture.multiply.frag.spv" render.frag

pushd "${TMPDIR}"
zip rd-shaders.pk3 spv/*
popd

rm -rf "${SPVDIR}"
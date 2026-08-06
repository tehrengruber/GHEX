#
# ghex-org
#
# Copyright (c) 2014-2023, ETH Zurich
# All rights reserved.
#
# Please, refer to the LICENSE file in the root directory.
# SPDX-License-Identifier: BSD-3-Clause
#
try:
    import cupy as cp
except ImportError:
    cp = None


# Implements Nvidia's CUDA stream protocol, see
# https://nvidia.github.io/cuda-python/cuda-core/latest/interoperability.html#cuda-stream-protocol
class CUDAStreamProtocolMock:
    def __init__(self, stream):
        self.stream = stream

    def __cuda_stream__(self):
        return 0, self.stream.ptr


# Exposes `.ptr` and nothing else, the way cupy streams did before they grew
# `__cuda_stream__`. Recent cupy streams implement the protocol, so `extract_cuda_stream`
# resolves them there and its `.ptr` fallback would otherwise never be reached.
class PtrOnlyStreamMock:
    def __init__(self, stream):
        self.ptr = stream.ptr


# `None` selects a plain exchange; the others select a scheduled exchange. Between them
# they cover all three branches of `extract_cuda_stream`: "default" the `nullptr` one,
# "protocol" the `__cuda_stream__` one, "ptr" the `.ptr` one. "null" and "non_blocking"
# are there for the stream semantics rather than the conversion.
STREAM_KINDS = (None, "default", "null", "non_blocking", "protocol", "ptr")


def make_stream(kind):
    # returns the object handed to ghex and the cupy stream it denotes; the two differ
    # for the kinds that do not pass a cupy stream through directly
    if kind == "default":
        return None, cp.cuda.Stream.null
    stream = cp.cuda.Stream(null=True) if kind == "null" else cp.cuda.Stream(non_blocking=True)
    if kind == "protocol":
        return CUDAStreamProtocolMock(stream), stream
    if kind == "ptr":
        return PtrOnlyStreamMock(stream), stream
    return stream, stream

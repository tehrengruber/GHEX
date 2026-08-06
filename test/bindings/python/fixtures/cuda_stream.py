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


# Exposes `.ptr` and nothing else, the way cupy streams do up to cupy 13.
class PtrOnlyStreamMock:
    def __init__(self, stream):
        self.ptr = stream.ptr


# `None` selects a plain exchange; the others select a scheduled exchange. Between them
# they cover all three branches of `extract_cuda_stream`: "default" the `nullptr` one,
# "protocol" the `__cuda_stream__` one, "ptr" the `.ptr` one. "null" and "non_blocking"
# are there for the stream semantics rather than the conversion.
#
# Both mocks are needed because a real cupy stream only reaches one branch, and which
# one depends on the installed version: cupy 13 streams expose `.ptr` alone, cupy 14
# streams also implement `__cuda_stream__` and so are resolved there instead. Without
# the mocks, whichever branch the installed cupy does not exercise goes untested.
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

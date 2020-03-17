/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrMeshDrawOp.h"

#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"

GrMeshDrawOp::GrMeshDrawOp(uint32_t classID) : INHERITED(classID) {}

void GrMeshDrawOp::onPrepare(GrOpFlushState* state) { this->onPrepareDraws(state); }

void GrMeshDrawOp::createProgramInfo(Target* target) {
    this->createProgramInfo(&target->caps(),
                            target->allocator(),
                            target->outputView(),
                            target->detachAppliedClip(),
                            target->dstProxyView());
}

// This onPrepareDraws implementation assumes the derived Op only has a single programInfo -
// which is the majority of the cases.
void GrMeshDrawOp::onPrePrepareDraws(GrRecordingContext* context,
                                     const GrSurfaceProxyView* outputView,
                                     GrAppliedClip* clip,
                                     const GrXferProcessor::DstProxyView& dstProxyView) {
    SkArenaAlloc* arena = context->priv().recordTimeAllocator();

    // This is equivalent to a GrOpFlushState::detachAppliedClip
    GrAppliedClip appliedClip = clip ? std::move(*clip) : GrAppliedClip();

    this->createProgramInfo(context->priv().caps(), arena, outputView,
                            std::move(appliedClip), dstProxyView);

    context->priv().recordProgramInfo(this->programInfo());
}

//////////////////////////////////////////////////////////////////////////////

GrMeshDrawOp::PatternHelper::PatternHelper(Target* target, GrPrimitiveType primitiveType,
                                           size_t vertexStride, sk_sp<const GrBuffer> indexBuffer,
                                           int verticesPerRepetition, int indicesPerRepetition,
                                           int repeatCount, int maxRepetitions) {
    this->init(target, primitiveType, vertexStride, std::move(indexBuffer), verticesPerRepetition,
               indicesPerRepetition, repeatCount, maxRepetitions);
}

void GrMeshDrawOp::PatternHelper::init(Target* target, GrPrimitiveType primitiveType,
                                       size_t vertexStride, sk_sp<const GrBuffer> indexBuffer,
                                       int verticesPerRepetition, int indicesPerRepetition,
                                       int repeatCount, int maxRepetitions) {
    SkASSERT(target);
    if (!indexBuffer) {
        return;
    }
    sk_sp<const GrBuffer> vertexBuffer;
    int firstVertex;
    int vertexCount = verticesPerRepetition * repeatCount;
    fVertices = target->makeVertexSpace(vertexStride, vertexCount, &vertexBuffer, &firstVertex);
    if (!fVertices) {
        SkDebugf("Vertices could not be allocated for patterned rendering.");
        return;
    }
    SkASSERT(vertexBuffer);
    fMesh = target->allocMesh();
    fPrimitiveType = primitiveType;

    SkASSERT(maxRepetitions ==
             static_cast<int>(indexBuffer->size() / (sizeof(uint16_t) * indicesPerRepetition)));
    fMesh->setIndexedPatterned(std::move(indexBuffer), indicesPerRepetition, repeatCount,
                               maxRepetitions, std::move(vertexBuffer), verticesPerRepetition,
                               firstVertex);
}

void GrMeshDrawOp::PatternHelper::recordDraw(Target* target, const GrGeometryProcessor* gp) const {
    target->recordDraw(gp, fMesh, 1, fPrimitiveType);
}

void GrMeshDrawOp::PatternHelper::recordDraw(
        Target* target,
        const GrGeometryProcessor* gp,
        const GrSurfaceProxy* const primProcProxies[]) const {
    target->recordDraw(gp, fMesh, 1, primProcProxies, fPrimitiveType);
}

//////////////////////////////////////////////////////////////////////////////

GrMeshDrawOp::QuadHelper::QuadHelper(Target* target, size_t vertexStride, int quadsToDraw) {
    sk_sp<const GrGpuBuffer> indexBuffer = target->resourceProvider()->refNonAAQuadIndexBuffer();
    if (!indexBuffer) {
        SkDebugf("Could not get quad index buffer.");
        return;
    }
    this->init(target, GrPrimitiveType::kTriangles, vertexStride, std::move(indexBuffer),
               GrResourceProvider::NumVertsPerNonAAQuad(),
               GrResourceProvider::NumIndicesPerNonAAQuad(), quadsToDraw,
               GrResourceProvider::MaxNumNonAAQuads());
}

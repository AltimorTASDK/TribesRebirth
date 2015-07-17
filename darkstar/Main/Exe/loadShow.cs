#
#
#
#
alias show "set ConsoleWorld::Echo"
alias showGfxSW show "'$ConsoleWorld::FrameRate' P:'$GFXMetrics::EmittedPolys','$GFXMetrics::RenderedPolys' S:'$GFXMetrics::UsedSpans' TSU:'$GFXMetrics::textureSpaceUsed' NLM:'$GFXMetrics::numLightMaps' NLMU:'$GFXMetrics::lightMapsInUse'"
alias showGfxHW show "'$ConsoleWorld::FrameRate' P:'$GFXMetrics::EmittedPolys','$GFXMetrics::RenderedPolys' TD:'$GFXMetrics::numTexturesDownloaded' TS:'$GFXMetrics::numTexturesScavenged' TBU:'$GFXMetrics::textureSpaceUsed' TBD:'$GFXMetrics::textureBytesDownloaded' LM:'$GFXMetrics::numLightMaps' LMU:'$GFXMetrics::lightMapsInUse'"
alias showTerrain show "S:'$GridMetrics::VisibleSquares' V:'$GridMetrics::TransformedVertices' P:'$GridMetrics::RenderedPolys'"
alias showNet show "Sent:'$SimRouter::sendCount' '$SimRouter::sendLastSize' '$SimRouter::sendTotalSize' '$SimRouter::sendErrors'  Recd:'$SimRouter::recvCount' '$SimRouter::recvLastSize' '$SimRouter::recvTotalSize' '$SimRouter::recvErrors'"
alias showNone show ""

#
#
#
#
alias show "set Console::LastLineTimeout 500; set ConsoleWorld::Echo "
alias showFPS   show "'$ConsoleWorld::FrameRate'"
alias showGfxSW show "'$ConsoleWorld::FrameRate' P:'$GFXMetrics::EmittedPolys','$GFXMetrics::RenderedPolys' S:'$GFXMetrics::UsedSpans' TSU:'$GFXMetrics::textureSpaceUsed'"
alias showGfxHW show "'$ConsoleWorld::FrameRate' P:'$GFXMetrics::RenderedPolys' NTD:'$GFXMetrics::numTexturesDownloaded' TSU:'$GFXMetrics::textureSpaceUsed' TBD:'$GFXMetrics::textureBytesDownloaded'"
alias showGfxGTX show "'$ConsoleWorld::FrameRate' NPD:'$GFXMetrics::numPaletteDLs' NTD:'$GFXMetrics::numTexturesDownloaded' WC0:'$GFXMetrics::numTMU0Wraps' WC1:'$GFXMetrics::numTMU1Wraps' TSU:'$GFXMetrics::textureSpaceUsed' TBD:'$GFXMetrics::textureBytesDownloaded'"
alias showTerrain show "S:'$GridMetrics::VisibleSquares' V:'$GridMetrics::TransformedVertices' P:'$GridMetrics::RenderedPolys'"
alias showNet show "Sent:'$SimRouter::sendCount' '$SimRouter::sendLastSize' '$SimRouter::sendTotalSize' '$SimRouter::sendErrors'  Recd:'$SimRouter::recvCount' '$SimRouter::recvLastSize' '$SimRouter::recvTotalSize' '$SimRouter::recvErrors'"
alias showITR show "'$ConsoleWorld::FrameRate' Rendered: '$ITRMetrics::NumRenderedInteriors' OutsideBits: '$ITRMetrics::OutsideBits'"
alias showNone show ""


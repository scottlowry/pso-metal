// P2-PREVIEW window-id probe — committed source of the /tmp/winlist.swift
// pattern used across the wave-5 fleet. Prints the FIRST on-screen window
// whose owner matches the pattern (or ships the pso-metal drivers):
//
//   winlist <pattern>   -> "<windowID>;<x>,<y>,<w>,<h>"   (or "none")
//   winlist --all       -> "<windowID>|<owner>|L<layer>|<w>x<h>@<x>,<y>" per line
//
// preview.sh feeds the ID to `screencapture -o -x -l<ID>`. CGWindowList and
// window-ID capture both require Screen Recording (TCC) in the launching
// context — preview.sh preflights that and fails LOUD (known mode), so a
// daemon-context denial never reads as a silent zero.
//
// Binary built by `make -C host/gx/preview` into build/gxpreview/bin/
// (gitignored); only this source is committed.
import CoreGraphics
import Foundation
let pat = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "gx_"
let opts: CGWindowListOption = [.optionOnScreenOnly, .excludeDesktopElements]
guard let info = CGWindowListCopyWindowInfo(opts, kCGNullWindowID) as? [[String: Any]] else { print("none"); exit(0) }
func bounds(_ w: [String: Any]) -> (Int, Int, Int, Int)? {
    guard let b = w[kCGWindowBounds as String] as? [String: Any],
          let x = b["X"] as? CGFloat, let y = b["Y"] as? CGFloat,
          let wd = b["Width"] as? CGFloat, let ht = b["Height"] as? CGFloat else { return nil }
    return (Int(x), Int(y), Int(wd), Int(ht))
}
if pat == "--all" {
    for w in info {
        let o = (w[kCGWindowOwnerName as String] as? String) ?? "?"
        let l = (w[kCGWindowLayer as String] as? Int) ?? -999
        let n = (w[kCGWindowNumber as String] as? Int) ?? -1
        if let (x, y, wd, ht) = bounds(w) { print("\(n)|\(o)|L\(l)|\(wd)x\(ht)@\(x),\(y)") }
    }
    exit(0)
}
for w in info {
    let owner = (w[kCGWindowOwnerName as String] as? String) ?? ""
    if owner.contains(pat) || owner.contains("pso-metal") {
        let n = (w[kCGWindowNumber as String] as? Int) ?? -1
        if let (x, y, wd, ht) = bounds(w) { print("\(n);\(x),\(y),\(wd),\(ht)") ; exit(0) }
    }
}
print("none")

importPackage(Packages.org.csstudio.opibuilder.scriptUtil);

var value = PVUtil.getLong(pvs[0]);
var n = widget.getMacroValue("N");
//ConsoleUtil.writeString("N: "+ n);
var btn = display.getWidget("Boolean Button " + n);
//ConsoleUtil.writeString("btn: "+ btn);

if (value & (1 << n)) {
	widget.setPropertyValue("text", "Input");
	// disable the set level button for input pins
	btn.setPropertyValue("enabled", false);
} else {
	widget.setPropertyValue("text", "Output");
}

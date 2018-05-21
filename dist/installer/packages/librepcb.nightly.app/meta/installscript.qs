function Component() {
}

Component.prototype.createOperations = function() {
    component.createOperations();

    try {
        if (systemInfo.productType === "windows") {
            component.addOperation("CreateShortcut",
                                   "@TargetDir@/nightly/bin/librepcb.exe", 
                                   "@StartMenuDir@/LibrePCB Nightly.lnk");
        }
    } catch (e) {
        print(e);
    }
}
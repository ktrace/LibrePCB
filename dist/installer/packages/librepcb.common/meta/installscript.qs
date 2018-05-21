function Component() {
}

Component.prototype.createOperations = function() {
    component.createOperations();

    try {
        if (systemInfo.productType === "windows") {
            component.addOperation("CreateShortcut",
                                   "@TargetDir@/librepcb-maintenance.exe", 
                                   "@StartMenuDir@/LibrePCB Maintenance Tool.lnk",
                                   " --updater");
        }
    } catch (e) {
        print(e);
    }
}
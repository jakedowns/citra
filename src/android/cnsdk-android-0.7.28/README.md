# CNSDK

CNSDK simplifies Leia Lightfield device support implementation.
It provides a simple API to set up and use all necessary components.

## File Structure Overview

### android

This folder contains CNSDK android archives (AARs). There are two variants of CNSDK:
* faceTrackingService - uses the tracking service. This is what should be used by default.
* faceTrackingInApp - does the tracking in the host app. This variant makes sense only when the host app requires access to the camera.

Additionally, this folder contains Javadoc-generated documentation. It can be opened directly (unzip it and open the index.html file) as well as included in Android Studio for quick documentation.

#### How to include javadoc.jar

1. In Android Studio make sure you are in the **Project** view, accessible from the pull-down menu above the navigation bar
    a. Find the library name under the **External Libraries** node (the Javadoc file appears below it).
    b. Right-click on the library name. Choose **Library Properties**
2. From the **Library Properties** dialog select the plus icon
    a. Navigate to the Javadoc JAR file and select it.
    b. Click **OK**.

### include

Native API headers. The API is unstable and backward compatibility is not guaranteed. Stable API and documentation are planned for future releases. The only source of documentation at the moment is sample projects (see samples/leia/lwEngine and the getting started projects).

### samples

See samples/README.md for build instructions.

Subtree structure overview:
/samples/android - Android project that contains all Android samples.
/samples/leia/sdk/tests - Java samples.
/samples/leia/lwEngine/tests - native samples.

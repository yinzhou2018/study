plugins { kotlin("multiplatform") }

repositories { mavenCentral() }

kotlin {
  macosArm64() {
    binaries { framework { baseName = "knative" } }
  }

  macosX64() {
    binaries { framework { baseName = "knative" } }
  }

  mingwX64()

  targets.withType<org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTarget> {
    binaries { sharedLib { baseName = "knative" } }
    
    binaries.all { 
      freeCompilerArgs += "-g"
      freeCompilerArgs += "-Xoverride-konan-properties=cxx_flang=-std=c++11"
    }
  }
}

val isWindows = org.gradle.internal.os.OperatingSystem.current().isWindows
val isX64 = System.getProperty("os.arch").matches(Regex("^(amd64|x86_64)$", RegexOption.IGNORE_CASE))
val targetName =  if (isWindows) "MingwX64" else if (isX64) "MacosX64" else "MacosArm64"
val folderName = if (isWindows) "mingwX64" else if (isX64) "macosX64" else "macosArm64"

tasks.register("buildDebugNativeApp") {
  group = "build"
  description = "Builds the native application."
  
  dependsOn("linkDebugShared$targetName")
  val targetDir = project.file("build/bin/$folderName/debugShared")
  val cmakeDir = project.file("build/cmake")
  cmakeDir.mkdirs()

  if (isWindows) {
    doFirst {
      exec {
        workingDir = targetDir
        commandLine("lib", "/def:knative.def", "/out:knative.lib")
      }
    }
  }

  doLast {
    exec {
      workingDir = cmakeDir
      commandLine("cmake", "-DOUTPUT_DIR=${targetDir.absolutePath}", "../../native")
    }
    exec {
      workingDir = cmakeDir
      commandLine("cmake", "--build", ".")
    }
  }
}

tasks.register("runDebugNativeApp") {
  group = "run"
  description = "Runs the native application."
  
  dependsOn("buildDebugNativeApp")

  doLast {
    exec {
      val targetDir = project.file("build/bin/$folderName/debugShared")
      workingDir = targetDir
      val executable = if (isWindows) "native_app.exe" else "native_app"
      commandLine("${targetDir.absolutePath}/$executable")
    }
  } 
}

if (!isWindows) {
  val targetDir = project.file("build/bin/$folderName/debugFramework")
  tasks.register("buildDebugMacosNativeApp") {
    group = "build"
    description = "Builds the macos native application."
    
    dependsOn("linkDebugFramework$targetName")

    val cmakeDir = project.file("build/macos_cmake")
    cmakeDir.mkdirs()
  
    doLast {
      exec {
        workingDir = cmakeDir
        commandLine("cmake", "-DOUTPUT_DIR=${targetDir.absolutePath}", "../../macos_native")
      }
      exec {
        workingDir = cmakeDir
        commandLine("cmake", "--build", ".")
      }
    }
  }
  
  tasks.register("runDebugMacosNativeApp") {
    group = "run"
    description = "Runs the macos native application."
    dependsOn("buildDebugMacosNativeApp")
  
    doLast {
      exec {
        workingDir = targetDir
        commandLine("${targetDir.absolutePath}/macos_native_app")
      }
    } 
  }
}
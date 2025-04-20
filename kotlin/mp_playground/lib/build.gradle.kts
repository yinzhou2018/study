plugins { kotlin("multiplatform") }

repositories { mavenCentral() }

kotlin {
  macosArm64() {
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

tasks.register("buildDebugNativeApp") {
  group = "build"
  description = "Builds the native application."
  
  val targetDir = if (isWindows) {
      dependsOn("linkDebugSharedMingwX64")
      project.file("build/bin/mingwX64/debugShared")
    } else {
      dependsOn("linkDebugSharedMacosArm64")
      project.file("build/bin/macosArm64/debugShared")
    }
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
  
  val isWindows = org.gradle.internal.os.OperatingSystem.current().isWindows

  val targetDir = if (isWindows) {
      project.file("build/bin/mingwX64/debugShared")
    } else {
      project.file("build/bin/macosArm64/debugShared")
    }

  dependsOn("buildDebugNativeApp")

  doLast {
    exec {
      workingDir = targetDir
      val executable = if (isWindows) "native_app.exe" else "native_app"
      commandLine("${targetDir.absolutePath}/$executable")
    }
  } 
}

if (!isWindows) {
  val targetDir = project.file("build/bin/macosArm64/debugFramework")
  tasks.register("buildDebugMacosNativeApp") {
    group = "build"
    description = "Builds the macos native application."
    
    dependsOn("linkDebugFrameworkMacosArm64")

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
plugins { kotlin("multiplatform") }

repositories { mavenCentral() }

kotlin {
  macosArm64()
  mingwX64()

  targets.withType<org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTarget> {
    binaries { sharedLib { baseName = "knative" } }

    binaries.all { freeCompilerArgs += "-g" }
  }
}

tasks.register("buildDebugNativeApp") {
  group = "build"
  description = "Builds the native application."
  
  val isWindows = org.gradle.internal.os.OperatingSystem.current().isWindows

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
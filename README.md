Onsem Android
=============

Android wrapper of onsem<br/>
Url of onsem: https://github.com/carloacu/onsem

### Step 1. Add the maven repository to your build file
Add it in your root build.gradle at the end of repositories:
```Kotlin
allprojects {
    repositories {
        ...
        maven { url 'https://raw.github.com/carloacu/onsem-android-releases/master' }
    }
}
```

### Step 2. Add the dependency
```Kotlin
dependencies {
    implementation 'com.github.carloacu:onsem-android:1.0.9'
}
```

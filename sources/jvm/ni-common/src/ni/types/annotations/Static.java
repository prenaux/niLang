// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
package ni.types.annotations;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 *
 * @author Samuel Audet
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.METHOD})
public @interface Static {
    /**
     * Replace the name of the static function with this value, this overrided the type as-well.
     */
    String name() default "";
    /**
     * Replace the static function call with this value.
     */
    String call() default "";
}

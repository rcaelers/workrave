// https://github.com/jhildenbiddle/mergician/issues/1

declare module 'mergician' {

  type ObjectLiteral = Record<any, any>

  export interface MergicianOptions {

    // ----
    // Keys
    // ----

    /**
     * Exclusive array of keys to be merged (others are skipped).
     */
    onlyKeys?: string[];

    /**
     * Array of keys to skip (others are merged).
     */
    skipKeys?: string[];

    /**
     * Merge only keys found in multiple objects (ignore single occurrence keys).
     */
    onlyCommonKeys?: boolean;

    /**
     * Merge only keys found in all objects
     */
    onlyUniversalKeys?: boolean;

    /**
     * Skip keys found in multiple objects (merge only single
     *     occurrence keys)
     */
    skipCommonKeys?: boolean;

    /**
     * Skip keys found in all objects (merge only common keys)
     */
    skipUniversalKeys?: boolean;

    // ------
    // Arrays
    // ------

    /**
     * Merge array values at the end of existing arrays
     */
    appendArrays?: boolean;

    /**
     * Merge array values at the beginning of existing arrays
     */
    prependArrays?: boolean;

    /**
     * Remove duplicate array values in new merged object
     */
    dedupArrays?: boolean;

    /**
     * Sort array values in new merged object
     */
    sortArrays?: boolean;

    // ---------
    // Callbacks
    // ---------

    /**
     * Callback used for inspecting/modifying properties before merge.
     * Return value is used as value to merge.
     */
    beforeEach?(options: { depth: number, key: string, srcObj: ObjectLiteral, srcVal: any, targetObj: ObjectLiteral, targetVal: any }): any | undefined;

    /**
     * Callback used to conditionally merge or skip a property.
     * Return "true" to merge or "false" to skip.
     * Return no value to proceed according to other option values.
     */
    filter?(options: { depth: number, key: string, srcObj: ObjectLiteral, srcVal: any, targetObj: ObjectLiteral, targetVal: any }): boolean;

    /**
     * Callback used for inspecting/modifying properties after merge.
     * Return value is used as merged value.
     */
    afterEach?<M>(options: { depth: number, key: string, mergeVal: M, srcObj: ObjectLiteral, targetObj: ObjectLiteral }): any | undefined;
  }

  /**
   * Deep recursive object merging with options to inspect, modify, and filter
   * keys/values, merge arrays (append/prepend), and remove duplicate values from
   * merged arrays. Returns new object without modifying sources (immutable).
   *
   * @preserve
   *
   * @example
   * const obj1 = { a: 1 };
   * const obj2 = { b: [2, 2], c: { d: 2 } };
   * const obj3 = { b: [3, 3], c: { e: 3 } };
   *
   * const mergedObj = mergician({
   *     appendArrays: true,
   *     dedupArrays: true
   * })(obj1, obj2, obj3);
   *
   * console.log(mergedObj); // { a: 1, b: [2, 3], c: { d: 2, e: 3 } }
   *
   * @param options
   */
  export default function mergician<T extends ObjectLiteral>(options: MergicianOptions): (...objects: ObjectLiteral[]) => T;

  /**
   * Deep recursive object merging with options to inspect, modify, and filter
   * keys/values, merge arrays (append/prepend), and remove duplicate values from
   * merged arrays. Returns new object without modifying sources (immutable).
   *
   * @preserve
   *
   * @example
   * const obj1 = { a: 1 };
   * const obj2 = { b: [2, 2], c: { d: 2 } };
   * const obj3 = { b: [3, 3], c: { e: 3 } };
   *
   * const customMerge = mergician({
   *     appendArrays: true,
   *     dedupArrays: true
   * });
   * const clonedObj = customMerge({}, obj2);
   * const mergedObj = customMerge(obj1, obj2, obj3);
   *
   * console.log(clonedObj);  // { b: [2], c: { d: 2 } }
   * console.log(mergedObj);  // { a: 1, b: [2, 3], c: { d: 2, e: 3 } }
   * @param options
   */
  export default function mergician<T extends ObjectLiteral>(options: MergicianOptions): (options: MergicianOptions) => (...objects: ObjectLiteral[]) => T;

  /**
   * Deep recursive object merging with options to inspect, modify, and filter
   * keys/values, merge arrays (append/prepend), and remove duplicate values from
   * merged arrays. Returns new object without modifying sources (immutable).
   *
   * @preserve
   *
   * @example
   * const obj1 = { a: 1 };
   * const obj2 = { b: [2, 2], c: { d: 2 } };
   * const obj3 = { b: [3, 3], c: { e: 3 } };
   *
   * const clonedObj = mergician({}, obj1);
   * const mergedObj = mergician(obj1, obj2, obj3);
   *
   * console.log(clonedObj);              // { a: 1 }
   * console.log(clonedObj === obj1);     // false
   * console.log(mergedObj);              // { a: 1, b: [3, 3], c: { d: 2, e: 3 } }
   *
   * @param {...object} [objects] - Objects to merge
   * @returns {object} New merged object
   */
  export default function mergician<T extends ObjectLiteral>(...objects: ObjectLiteral[]): T;
}

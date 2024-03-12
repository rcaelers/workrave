/**
 * @description Deep (recursive) object merging with support for descriptor
 * values, accessor functions, custom prototypes, and advanced options for
 * customizing the merge process.
 *
 * @example
 * // Custom merge options
 * const mergedObj = mergician({
 *   // Options
 * })(obj1, obj2, obj3);
 *
 * // Custom merge function
 * const customMerge = mergician({
 *   // Options
 * });
 * const customMergeObj = customMerge(obj1, obj2, obj3);
 *
 * @overload
 * @param {MergicianOptions} options
 * @returns {function} New merge function with options set as defaults
 * @preserve
 */
export function mergician(options: MergicianOptions): Function;
/**
 * @description Deep (recursive) object merging with support for descriptor
 * values, accessor functions, custom prototypes, and advanced options for
 * customizing the merge process.
 *
 * @example
 * // Clone with default options
 * const clonedObj = mergician({}, obj1);
 *
 * // Merge with default options
 * const mergedObj = mergician(obj1, obj2, obj3);
 *
 * @overload
 * @param {...object} objects
 * @returns {object} New merged object
 * @preserve
 */
export function mergician(...objects: object[]): object;
export type MergicianOptions = {
    /**
     * - Exclusive array of keys to be merged
     * (others are skipped)
     */
    onlyKeys?: string[];
    /**
     * - Array of keys to skip (others are
     * merged)
     */
    skipKeys?: string[];
    /**
     * - Merge only keys found
     * in multiple objects (ignore single occurrence keys)
     */
    onlyCommonKeys?: boolean;
    /**
     * - Merge only keys
     * found in all objects
     */
    onlyUniversalKeys?: boolean;
    /**
     * - Skip keys found in
     * multiple objects (merge only single occurrence keys)
     */
    skipCommonKeys?: boolean;
    /**
     * - Skip keys found in
     * all objects (merge only common keys)
     */
    skipUniversalKeys?: boolean;
    /**
     * - Invoke "getter" methods
     * and merge returned values
     */
    invokeGetters?: boolean;
    /**
     * - Skip "setter" methods
     * during merge
     */
    skipSetters?: boolean;
    /**
     * - Merge array values at
     * the end of existing arrays
     */
    appendArrays?: boolean;
    /**
     * - Merge array values at
     * the beginning of existing arrays
     */
    prependArrays?: boolean;
    /**
     * - Remove duplicate array
     * values in new merged object
     */
    dedupArrays?: boolean;
    /**
     * - Sort array values
     * in new merged object
     */
    sortArrays?: boolean | Function;
    /**
     * - Merge enumerable
     * prototype properties as direct properties of merged object
     */
    hoistEnumerable?: boolean;
    /**
     * - Merge custom prototype
     * properties as direct properties of merged object
     */
    hoistProto?: boolean;
    /**
     * - Skip merging of custom
     * prototype properties
     */
    skipProto?: boolean;
    /**
     * - Callback used to conditionally merge
     * or skip a property. Return a "truthy" value to merge or a "falsy" value to
     * skip. Return no value to proceed according to other option values.
     */
    filter?: filterCallback;
    /**
     * - Callback used for
     * inspecting/modifying properties before merge. Return value is used as value
     * to merge.
     */
    beforeEach?: beforeEachCallback;
    /**
     * - Callback used for
     * inspecting/modifying properties after merge. Return value is used as merged
     * value.
     */
    afterEach?: afterEachCallback;
    /**
     * - Callback used for handling
     * circular object references during merge
     */
    onCircular?: onCircularCallback;
};
export type filterCallback = (callbackData: callbackData) => any;
export type beforeEachCallback = (callbackData: callbackData) => any;
export type afterEachCallback = (callbackData: afterEachCallbackData) => any;
export type onCircularCallback = (callbackData: callbackData) => any;
export type callbackData = {
    /**
     * - Nesting level of the key being processed
     */
    depth: number;
    /**
     * - Object key being processed
     */
    key: string;
    /**
     * - Object containing the source value
     */
    srcObj: object;
    /**
     * - Source object’s property value
     */
    srcVal: any;
    /**
     * - New merged object
     */
    targetObj: object;
    /**
     * - New merged object’s current property value
     */
    targetVal: any;
};
export type afterEachCallbackData = {
    /**
     * - Nesting level of the key being processed
     */
    depth: number;
    /**
     * - Object key being processed
     */
    key: string;
    /**
     * - New merged value
     */
    mergeVal: any;
    /**
     * - Object containing the source value
     */
    srcObj: object;
    /**
     * - New merged object
     */
    targetObj: object;
};

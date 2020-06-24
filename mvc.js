
Angstrom = (() => {
    "use strict";
    /**
     * Functional closure for dot string access of contexts.
     * @param  {string} attr Dot-delimted attribute accessor
     * @returns  {(ctxt:Object)=>Object} A funtion to access `attr` attribute of `ctxt`
     */
    function fromDot(attr) {
        return ctxt => attr.split('.').reduce((v, a) => v[a] || v, ctxt);
    };

    /**
     * Converts `html` to an equivalent Node representation.
     * @param  {string} html HTML content string
     * @returns  {NodeListOf<ChildNode>} The node content representation.
     */
    function htmlToNodes(html) {
        let template = document.createElement('template');
        template.innerHTML = html.trim();
        return template.content.childNodes;
    };

    /**
     * Searches for (finds) elements meeting a predicate.
     * If an element is added, its children are not searched further.
     * @param  {Node[]} elements The elements from which to find included elements or children elements.
     * @param  {(el:Node)=>bool} predicate A filter method to select included nodes
     * `predicate` returns true on inclusion.
     * @returns  {Node[]} The Included (found) elements
     */
    function find(elements, predicate) {
        if (elements.length == 0) return [];

        const
            not = (el) => !predicate(el),
            included = elements.filter(predicate),
            furtherSearch = elements.filter(not);

        return [].concat.apply(included, furtherSearch.map(el => find([...el.children], predicate)));
    };

    /**
     * Extends a prototype onto the caller.
     * @param {Object} proto The prototype to extend.
     */
    function extend(proto) {
        Object.keys(proto).forEach(attr => this[attr] = proto[attr]);
    }

    class Model {
        /**
         * Model contains the state of the page,
         * along with references to which nodes need to be updated.
         * @param {Object} state the model's `$state`
         */
        constructor(state) {
            this.$state = state;
            this._observers = [];
        }


        /**
         * Notifies all observers to an update of `$state`.
         */
        notifyAll() {
            this._observers.forEach((obs) => obs.update(this.$state, model));
        }

        /**
         * Adds an observer to be notified later.
         * @param {{update: (state: Object, model: Model)=>*}} observer the observer to Add.
         * The observer must have a method called `update` to be called on notification.
         */
        registerObserver(observer) {
            this._observers.push(observer);
        }
    }

    class View {
        /**
         * View is a render system for a controlled element.
         * @param {Node} element The Container element.
         * @param {Model} model The state model.
         * @param {Controller} controller The action controller for this view.
         * @param {(values:Object)=>string} compute The method to render the context computation.
         */
        constructor(element, model, controller, compute) {
            var self = this;
            self._element = element;
            self._model = model;
            self._controller = controller;
            if ('am-bind' in element.attributes) {
                let key = element.attributes['am-bind'].value;
                element.onchange = function () {
                    self._model.$state[key] = element.value;
                    self._model.notifyAll();
                }
            }
            if ('am-for' in element.attributes) {
                let key = element.attributes['am-for'].value.trim();
                let [item, collection] = key.split(' in ', 2);
                self.collection = collection;
                self.item = item;
            }

            Array.from(element.attributes).forEach(attr => {
                if (/^am-on-/.test(attr.nodeName)) {
                    let event = attr.name.replace(/^am-on-/, '');
                    let funcName = attr.value.trim();
                    element.addEventListener(event, evt => {
                        if (evt.type === event) {
                            controller[funcName](model.$state, model, element);
                        }
                    });
                }
            });
            this._compute = compute;
        }

        /**
         * Updates the rendering from the computed context values.
         * All of `_element`'s children Nodes are removed before rendering.
         * If the view is part of a collection (`am-for`), the compute routine is iterated over the collection.
         * @param {Object} state The context state values.
         */
        update(state) {
            let self = this;

            while (self._element.hasChildNodes()) {
                self._element.removeChild(self._element.firstChild);
            }

            if (self.collection !== undefined && self.item !== undefined) {
                const mapping = state[self.collection]
                    .map(value => self._compute(Object.fromEntries([[self.item, value]])))
                    .join('');
                htmlToNodes(mapping).forEach(node => self._element.appendChild(node));
            } else {
                htmlToNodes(self._compute(state)).forEach(node => self._element.appendChild(node));
            }
        }
    }

    class Controller {
        /**
         * Controller houses the logic of acting on data change and view rendering.
         * It should control when, and if views render after data changes.
         * @param {string} name Controller registration name.
         * @param {Model} model Data Model for this controller.
         * @param {Object} proto Controller logic prototype.
         */
        constructor(name, model, proto) {
            var self = this;
            this.name = name;
            this._model = model;
            this._elements = find([...document.children], el => (el.attributes['am-controller'] || {}).value == name);
            this._views = this._elements.map(el => self.compile(el, this._model));

            this._views.forEach((view) => this._model.registerObserver(view));
            this._model.registerObserver(this);

            if (proto !== undefined) {
                extend.call(this, proto);
            }

            this.update = Controller.prototype.update;
        }

        /**
         * A notification update wrapper for `Model`.
         * @param {Object} state The current model `$state`.
         * @param {Model} model The base model.
         */
        update(state, model) {
            if (this.$update !== undefined)
                this.$update(state, model, this._elements);
        }

        /**
         * Creates a view with a compute method from the element template.
         * Derived from Angular 1 interpolate function:
         * See: https://github.com/angular/angular.js/blob/master/src/ng/interpolate.js#L240
         * @param {Node} element The element that contains the render template and render bindings
         * @param {Model} model The binding state model.
         */
        compile(element, model) {
            const text = element.innerHTML;
            var self = this;
            var startIndex,
                endIndex,
                index = 0,
                expressions = [],
                parseFns,
                textLength = text.length,
                exp,
                concat = [],
                expressionPositions = [];
            while (index < textLength) {
                if (((startIndex = text.indexOf('{{', index)) !== -1) && ((endIndex = text.indexOf('}}', index + 2)) !== -1)) {
                    if (index != startIndex) {
                        concat.push(text.substring(index, startIndex));
                    }
                    exp = text.substring(startIndex + 2, endIndex).trim(' ');
                    expressions.push(exp);
                    index = endIndex + 2;
                    expressionPositions.push(concat.length);
                    concat.push('');
                } else {
                    if (index !== textLength) {
                        concat.push(text.substring(index));
                    }
                    break;
                }
            }
            parseFns = expressions.map(fromDot);
            const compute = values => { expressionPositions.map((p, i) => concat[p] = parseFns[i](values)); return concat.join(''); };
            return new View(element, model, self, compute);
        }
    }

    let Angstrom = {};
    Angstrom.Controller = Controller;
    Angstrom.Model = Model;
    Angstrom.View = View;

    return Angstrom;
})();
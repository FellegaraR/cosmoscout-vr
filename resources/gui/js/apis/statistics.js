class StatisticsApi extends IApi {
  /**
   * @inheritDoc
   */
  name = 'statistics';

  _values = [];

  _data;

  _colorHash;

  _minTime = 1000;

  _maxValue = 1e9 / 30;

  _alpha = 0.95;

  init() {
    if (typeof ColorHash !== 'undefined') {
      this._colorHash = new ColorHash({ lightness: 0.5, saturation: 0.3 });
    } else {
      console.error('Class \'ColorHash\' not defined.');
    }
  }

  /**
   *
   * @param data {string}
   * @param frameRate {number}
   */
  setData(data, frameRate) {
    this._data = JSON.parse(data);

    // first set all times to zero
    this._resetTimes();

    // then add all new elements
    this._addNewElements();

    // remove all with very little contribution
    this._values = this._values.filter((element) => element.timeGPU > this._minTime || element.timeCPU > this._minTime
      || element.avgTimeGPU > this._minTime || element.avgTimeCPU > this._minTime);

    // update average values
    this._values.forEach((element) => {
      element.avgTimeGPU = element.avgTimeGPU * this._alpha + element.timeGPU * (1 - this._alpha);
      element.avgTimeCPU = element.avgTimeCPU * this._alpha + element.timeCPU * (1 - this._alpha);
    });

    // sort by average
    this._values.sort((a, b) => (b.avgTimeGPU + b.avgTimeCPU) - (a.avgTimeGPU + a.avgTimeCPU));

    this._insertHtml(frameRate);
  }

  _resetTimes() {
    this._values.forEach(value => {
      if (typeof this._data[value.name] !== 'undefined') {
        value.timeGPU = this._data[value.name][0];
        value.timeCPU = this._data[value.name][1];
        this._data[value.name][0] = -1;
        this._data[value.name][1] = -1;
      } else {
        value.timeGPU = 0;
        value.timeCPU = 0;
      }
    });
  }

  _addNewElements() {
    for (const key in this._data) {
      if (!this._data.hasOwnProperty(key)) {
        continue;
      }

      if (this._data[key][0] >= 0) {
        this._values.push({
          name: key,
          timeGPU: this._data[key][0],
          timeCPU: this._data[key][1],
          avgTimeGPU: this._data[key][0],
          avgTimeCPU: this._data[key][1],
          color: this._colorHash.hex(key),
        });
      }
    }
  }

  _insertHtml(frameRate) {
    const container = document.getElementById('statistics');
    CosmoScout.clearHtml(container);

    const maxEntries = Math.min(10, this._values.length);
    const maxWidth = container.offsetWidth;

    const item = document.createElement('template');

    item.innerHTML = `<div class="label"><strong>FPS: ${frameRate.toFixed(2)}</strong></div>`;

    container.appendChild(item.content);

    for (let i = 0; i < maxEntries; ++i) {
      const widthGPU = maxWidth * this._values[i].avgTimeGPU / this._maxValue;
      const widthCPU = maxWidth * this._values[i].avgTimeCPU / this._maxValue;

      item.innerHTML += `<div class="statistics-item">
        <div class="bar gpu" style="background-color:${this._values[i].color}; width:${widthGPU}px"><div class="label">gpu: ${(this._values[i].avgTimeGPU * 0.000001).toFixed(1)} ms</div></div>
        <div class="bar cpu" style="background-color:${this._values[i].color}; width:${widthCPU}px"><div class="label">cpu: ${(this._values[i].avgTimeCPU * 0.000001).toFixed(1)} ms</div></div>
        <div class="label">${this._values[i].name}</div>
      </div>`;

      container.appendChild(item.content);
    }
  }
}
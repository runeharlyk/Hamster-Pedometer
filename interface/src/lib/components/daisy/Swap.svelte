<script lang="ts">
	export let variant: 'normal' | 'flip' | 'rotate' = 'normal';
	export let checked = false;
</script>

<label
	class="swap {$$restProps.class}"
	class:swap-flip={variant === 'flip'}
	class:swap-rotate={variant === 'rotate'}
>
	<input type="checkbox" bind:checked />
	<div class="swap-on"><slot name="on" /></div>
	<div class="swap-off"><slot name="off" /></div>
</label>
